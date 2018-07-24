/*
Copyright (c) by respective owners including Yahoo!, Microsoft, and
individual contributors. All rights reserved.  Released under a BSD
license as described in the file LICENSE.
*/
#pragma once

typedef unsigned char namespace_index;

#include "v_array.h"
#include "feature_group.h"

struct example_predict
{
  class iterator
  {
    features* _feature_space;
    namespace_index* _index;
  public:
    iterator(features* feature_space, namespace_index* index)
      : _feature_space(feature_space), _index(index)
    { }

    features& operator*()
    {
      return _feature_space[*_index];
    }

    iterator& operator++()
    {
      _index++;
      return *this;
    }

    namespace_index index() { return *_index; }

    bool operator==(const iterator& rhs) { return _index == rhs._index; }
    bool operator!=(const iterator& rhs) { return _index != rhs._index; }
  };

  v_array<namespace_index> indices;
  features feature_space[256]; //Groups of feature values.
  uint64_t ft_offset;//An offset for all feature values.

  iterator begin() { return iterator(feature_space, indices.begin()); }
  iterator end() { return iterator(feature_space, indices.end()); }
};

// make sure we have an exception safe version of example_predict
class safe_example_predict : public example_predict
{
public:
  safe_example_predict();
  ~safe_example_predict();
};
