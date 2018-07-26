#include <cstring>
#include <float.h>
#include <math.h>
#include <stdio.h>

#include "cache.h"
#include "accumulate.h"
#include "best_constant.h"

using namespace std;

namespace no_label
{
  char* bufread_no_label(shared_data* sd, label_data* ld, char* c) { return c; }
  
  size_t read_cached_no_label(shared_data* sd, void* v, io_buf& cache) { return 1; }
  
  float get_weight(void* v) { return 1.; }
  
  char* bufcache_no_label(label_data* ld, char* c) { return c;  }
  
  void cache_no_label(void* v, io_buf& cache) { }
  
  void default_no_label(void* v) { }
  
  bool test_label(void* v) { return false; }
  
  void delete_no_label(void*) {}
  
  void parse_no_label(parser*, shared_data* sd, void* v, v_array<substring>& words)
  {
    switch(words.size())
      {
      case 0:
        break;
      default:
        cout << "Error: " << words.size() << " is too many tokens for a simple label: ";
        for(unsigned int i=0; i<words.size(); ++i)
          print_substring(words[i]);
        cout << endl;
      }
  }
  
  label_parser no_label_parser = {default_no_label, parse_no_label,
                                  cache_no_label, read_cached_no_label,
                                  delete_no_label, get_weight,
                                  nullptr,
                                  test_label,
                                  sizeof(nullptr)
  };
  
  void print_no_label_update(vw& all, example& ec)
  {
    if (all.sd->weighted_labeled_examples + all.sd->weighted_unlabeled_examples >= all.sd->dump_interval && !all.quiet && !all.bfgs)
      {
        all.sd->print_update(all.holdout_set_off, all.current_pass, 0.f, ec.pred.scalar,
                             ec.num_features, all.progress_add, all.progress_arg);
      }
  }
  
  void output_and_account_no_label_example(vw& all, example& ec)
  {
    all.sd->update(ec.test_only, false, ec.loss, ec.weight, ec.num_features);
    
    all.print(all.raw_prediction, ec.partial_prediction, -1, ec.tag);
    for (size_t i = 0; i<all.final_prediction_sink.size(); i++)
      {
        int f = (int)all.final_prediction_sink[i];
        all.print(f, ec.pred.scalar, 0, ec.tag);
      }
    
    print_no_label_update(all, ec);
  }
  
  void return_no_label_example(vw& all, void*, example& ec)
  {
    output_and_account_example(all, ec);
    VW::finish_example(all,ec);
  }
}
