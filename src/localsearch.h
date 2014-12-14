/**
 * Copyright 2014 Kevin Bader.
 * Distributed under the GNU GPL v3. For full terms see the file /LICENSE.
 */
#ifndef D2_LOCALSEARCH_H
#define D2_LOCALSEARCH_H

#include <ctime>
#include <random>
#include <common.h>
#include <graph.h>

namespace d2 {
namespace LS {

void minimalConflictsTreeSearch(ordering_t* ordering, Graph& graph,
                                std::default_random_engine& randgen,
                                unsigned maxNonimprovements, const clock_t maxTick,
                                unsigned* finalTreewidth);

unsigned ILS(ordering_t* ordering, Graph& graph,
             std::default_random_engine& randgen,
             unsigned maxOuterNonimprovements,
             unsigned maxInnerNonimprovements,
             const clock_t maxTick);

}  // ns LS
}  // ns d2

#endif  // D2_LOCALSEARCH_H
