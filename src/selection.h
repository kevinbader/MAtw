/**
 * Copyright 2014 Kevin Bader.
 * Distributed under the GNU GPL v3. For full terms see the file /LICENSE.
 */
#ifndef D2_SELECTION_H
#define D2_SELECTION_H

#include <random>
#include <common.h>
#include <graph.h>

namespace d2 {
namespace selection {

const ordering_t * k_tournament(const population_t& population, unsigned k,
                                Graph& graph,
                                std::default_random_engine& randgen);

}  // ns selection
}  // ns d2

#endif  // D2_SELECTION_H
