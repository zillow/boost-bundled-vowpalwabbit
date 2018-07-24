#include "gd.h"
#include "float.h"
#include "reductions.h"

using namespace std;
struct print { vw* all; }; //regressor, feature loop

void print_feature(vw& all, float value, uint64_t index)
{
  cout << index;
  if (value != 1.)
    cout << ":" << value;
  cout << " ";
}

void learn(print& p, LEARNER::base_learner&, example& ec)
{
  label_data& ld = ec.l.simple;
  if (ld.label != FLT_MAX)
  {
    cout << ld.label << " ";
    if (ec.weight != 1 || ld.initial != 0)
    {
      cout << ec.weight << " ";
      if (ld.initial != 0)
        cout << ld.initial << " ";
    }
  }
  if (ec.tag.size() > 0)
  {
    cout << '\'';
    cout.write(ec.tag.begin(), ec.tag.size());
  }
  cout << "| ";
  GD::foreach_feature<vw, uint64_t, print_feature>(*(p.all), ec, *p.all);
  cout << endl;
}

LEARNER::base_learner* print_setup(arguments& arg)
{
  if (arg.new_options("Print psuedolearner").critical("print", "print examples").missing())
    return nullptr;

  auto p = scoped_calloc_or_throw<print>();
  p->all = arg.all;

  arg.all->weights.stride_shift(0);

  LEARNER::learner<print,example>& ret = init_learner(p, learn, learn, 1);
  return make_base(ret);
}
