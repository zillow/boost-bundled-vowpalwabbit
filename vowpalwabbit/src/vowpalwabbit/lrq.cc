#include <string.h>
#include <float.h>
#include "reductions.h"
#include "rand48.h"
#include "vw_exception.h"
#include "parse_args.h" // for spoof_hex_encoded_namespaces

using namespace LEARNER;
using namespace std;

struct LRQstate
{
  vw* all; // feature creation, audit, hash_inv
  bool lrindices[256];
  size_t orig_size[256];
  std::set<std::string> lrpairs;
  bool dropout;
  uint64_t seed;
  uint64_t initial_seed;
};

bool valid_int (const char* s)
{
  char* endptr;

  int v = strtoul (s, &endptr, 0);
  (void) v;

  return (*s != '\0' && *endptr == '\0');
}

inline bool
cheesyrbit (uint64_t& seed)
{
  return merand48 (seed) > 0.5;
}

inline float
cheesyrand (uint64_t x)
{
  uint64_t seed = x;

  return merand48 (seed);
}

inline bool
example_is_test (example& ec)
{
  return ec.l.simple.label == FLT_MAX;
}

void
reset_seed (LRQstate& lrq)
{
  if (lrq.all->bfgs)
    lrq.seed = lrq.initial_seed;
}

template <bool is_learn>
void predict_or_learn(LRQstate& lrq, single_learner& base, example& ec)
{
  vw& all = *lrq.all;

  // Remember original features

  memset (lrq.orig_size, 0, sizeof (lrq.orig_size));
  for (namespace_index i : ec.indices)
  {
    if (lrq.lrindices[i])
      lrq.orig_size[i] = ec.feature_space[i].size ();
  }

  size_t which = ec.example_counter;
  float first_prediction = 0;
  float first_loss = 0;
  float first_uncertainty = 0;
  unsigned int maxiter = (is_learn && ! example_is_test (ec)) ? 2 : 1;

  bool do_dropout = lrq.dropout && is_learn && ! example_is_test (ec);
  float scale = (! lrq.dropout || do_dropout) ? 1.f : 0.5f;

  uint32_t stride_shift = lrq.all->weights.stride_shift();
  for (unsigned int iter = 0; iter < maxiter; ++iter, ++which)
  {
    // Add left LRQ features, holding right LRQ features fixed
    //     and vice versa
    // TODO: what happens with --lrq ab2 --lrq ac2
    //       i.e. namespace occurs multiple times (?)

    for (string const& i : lrq.lrpairs)
    {
      unsigned char left = i[which%2];
      unsigned char right = i[(which+1)%2];
      unsigned int k = atoi (i.c_str () + 2);

      features& left_fs = ec.feature_space[left];
      for (unsigned int lfn = 0; lfn < lrq.orig_size[left]; ++lfn)
      {
        float lfx = left_fs.values[lfn];
        uint64_t lindex = left_fs.indicies[lfn] + ec.ft_offset;
        for (unsigned int n = 1; n <= k; ++n)
        {
          if (! do_dropout || cheesyrbit (lrq.seed))
          {
            uint64_t lwindex = (lindex + ((uint64_t)n << stride_shift));
            weight* lw = &lrq.all->weights[lwindex];

            // perturb away from saddle point at (0, 0)
            if (is_learn && ! example_is_test (ec) && *lw == 0)
              *lw = cheesyrand (lwindex); //not sure if lw needs a weight mask?

            features& right_fs = ec.feature_space[right];
            for (unsigned int rfn = 0;
                 rfn < lrq.orig_size[right];
                 ++rfn)
            {
              // NB: ec.ft_offset added by base learner
              float rfx = right_fs.values[rfn];
              uint64_t rindex = right_fs.indicies[rfn];
              uint64_t rwindex = (rindex + ((uint64_t)n << stride_shift));

              right_fs.push_back(scale **lw * lfx * rfx, rwindex);

              if (all.audit || all.hash_inv)
              {
                std::stringstream new_feature_buffer;
                new_feature_buffer << right << '^'
                                   << right_fs.space_names[rfn].get()->second << '^'
                                   << n;

#ifdef _WIN32
                char* new_space = _strdup("lrq");
                char* new_feature =	_strdup(new_feature_buffer.str().c_str());
#else
                char* new_space = strdup("lrq");
                char* new_feature = strdup(new_feature_buffer.str().c_str());
#endif
                right_fs.space_names.push_back(audit_strings_ptr(new audit_strings(new_space,new_feature)));
              }
            }
          }
        }
      }
    }

    if (is_learn)
      base.learn(ec);
    else
      base.predict(ec);

    // Restore example
    if (iter == 0)
    {
      first_prediction = ec.pred.scalar;
      first_loss = ec.loss;
      first_uncertainty = ec.confidence;
    }
    else
    {
      ec.pred.scalar = first_prediction;
      ec.loss = first_loss;
      ec.confidence = first_uncertainty;
    }

    for (string const& i : lrq.lrpairs)
    {
      unsigned char right = i[(which+1)%2];
      ec.feature_space[right].truncate_to(lrq.orig_size[right]);
    }
  }
}

void finish(LRQstate& lrq) { lrq.lrpairs.~set<string>(); }

base_learner* lrq_setup(arguments& arg)
{
  auto lrq = scoped_calloc_or_throw<LRQstate>();
  if (arg.new_options("Low Rank Quadratics")
      .critical_vector<string>("lrq", po::value<vector<string> >(), "use low rank quadratic features")
      .keep(lrq->dropout, "lrqdropout", "use dropout training for low rank quadratic features").missing())
    return nullptr;

  uint32_t maxk = 0;
  lrq->all = arg.all;

  vector<string> lrq_names = arg.vm["lrq"].as<vector<string> > ();
  for (size_t i = 0; i < lrq_names.size(); i++) lrq_names[i] = spoof_hex_encoded_namespaces( lrq_names[i] );

  new(&lrq->lrpairs) std::set<std::string> (lrq_names.begin(), lrq_names.end());

  lrq->initial_seed = lrq->seed = arg.all->random_seed | 8675309;

  if (! arg.all->quiet)
  {
    arg.trace_message << "creating low rank quadratic features for pairs: ";
    if (lrq->dropout)
      arg.trace_message << "(using dropout) ";
  }

  for (string const& i : lrq->lrpairs)
  {
    if(!arg.all->quiet)
      {
        if (( i.length() < 3 ) || ! valid_int (i.c_str () + 2))
          THROW("error, low-rank quadratic features must involve two sets and a rank.");

        arg.trace_message << i << " ";
      }
    // TODO: colon-syntax

    unsigned int k = atoi (i.c_str () + 2);

    lrq->lrindices[(int) i[0]] = 1;
    lrq->lrindices[(int) i[1]] = 1;

    maxk = max (maxk, k);
  }

  if(!arg.all->quiet)
    arg.trace_message<<endl;

  arg.all->wpp = arg.all->wpp * (uint64_t)(1 + maxk);
  learner<LRQstate,example>& l = init_learner(lrq, as_singleline(setup_base(arg)), predict_or_learn<true>,
                                      predict_or_learn<false>, 1 + maxk);
  l.set_end_pass(reset_seed);
  l.set_finish(finish);

  // TODO: leaks memory ?
  return make_base(l);
}
