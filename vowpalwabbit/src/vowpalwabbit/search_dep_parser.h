/*
Copyright (c) by respective owners including Yahoo!, Microsoft, and
individual contributors. All rights reserved.  Released under a BSD
license as described in the file LICENSE.
 */
#pragma once
#include "search.h"

namespace DepParserTask
{
void initialize(Search::search&, size_t&, arguments&);
void finish(Search::search&);
void run(Search::search&, multi_ex&);
void setup(Search::search&, multi_ex&);
extern Search::search_task task;
}
