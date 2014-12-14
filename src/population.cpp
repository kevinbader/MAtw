/**
 * Copyright 2014 Kevin Bader.
 * Distributed under the GNU GPL v3. For full terms see the file /LICENSE.
 */
#include "population.h"

namespace d2 {

void initialize_random_ordering(std::default_random_engine& randgen,
                                ordering_t* ordering, unsigned size) {
    ordering->resize(size);
    std::iota(ordering->begin(), ordering->end(), 0);
    std::shuffle(ordering->begin(), ordering->end(), randgen);
}

population_t generate_random_population(std::default_random_engine& randgen,
                                        unsigned nIndividuals, unsigned nVertices) {
    population_t population;
    population.reserve(nIndividuals);
    for (auto i = 0u; i < nIndividuals; ++i) {
        ordering_t individual;
        initialize_random_ordering(randgen, &individual, nVertices);
        population.push_back(std::move(individual));
    }
    return population;
}

void find_best_individual(Graph& graph, const population_t& population,
                          const ordering_t **const best, unsigned *const bestTreewidth) {
    const ordering_t * best_ = nullptr;
    unsigned bestTreewidth_ = graph.nVertices();

    for (const auto& individual: population) {
        unsigned treewidth;
        graph.evaluate(individual, &treewidth);

        if (treewidth < bestTreewidth_) {
            best_ = &individual;
            bestTreewidth_ = treewidth;
        }
    }

    if (best != nullptr)
        *best = best_;
    if (bestTreewidth != nullptr)
        *bestTreewidth = bestTreewidth_;
}

}  // ns d2
