/*
Copyright (c) by respective owners including Yahoo!, Microsoft, and
individual contributors. All rights reserved.  Released under a BSD
license as described in the file LICENSE.
 */
#pragma once

namespace CSOAA {
  LEARNER::base_learner* csoaa_setup(arguments& arg);

  LEARNER::base_learner* csldf_setup(arguments& arg);
  struct csoaa;
  void finish_example(vw& all, csoaa&, example& ec);
}

namespace LabelDict
{
bool ec_is_example_header(example& ec);// example headers look like "0:-1" or just "shared"
}
