/**
 * Copyright 2014 Kevin Bader.
 * Distributed under the GNU GPL v3. For full terms see the file /LICENSE.
 */
#ifndef D2_POPULATION_H
#define D2_POPULATION_H

#include <common.h>
#include <graph.h>

namespace d2 {

void initialize_random_ordering(std::default_random_engine& randgen,
                                ordering_t* ordering, unsigned size);

population_t generate_random_population(std::default_random_engine& randgen,
                                        unsigned nIndividuals, unsigned nVertices);

void find_best_individual(Graph& graph, const population_t& population,
                          const ordering_t **const best, unsigned *const bestTreewidth);

}  // ns d2

#endif  // D2_POPULATION_H
