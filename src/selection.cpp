/**
 * Copyright 2014 Kevin Bader.
 * Distributed under the GNU GPL v3. For full terms see the file /LICENSE.
 */
#include "selection.h"

namespace d2 {
namespace selection {

vector<size_t> select_k_random_positions(const population_t& population, unsigned k,
                                         std::default_random_engine& randgen) {
    const auto populationSize = population.size();
    assert(0 < k and k <= populationSize);
    auto dist = std::uniform_int_distribution<size_t>(0, populationSize - 1);
    auto candidatePositions = vector<size_t>(k);
    for (auto i = 0u; i < k; ++i) {
        size_t candidate;
        bool isUnique;
        do {
            candidate = dist(randgen);
            isUnique = true;
            for (auto j = 0u; j < i; ++j) {
                if (candidate == candidatePositions[j]) {
                    isUnique = false;
                    break;  // looking for the candidate in candidatePositions
                }
            }
        } while (not isUnique);
        candidatePositions[i] = candidate;
    }
    return candidatePositions;
}

const ordering_t * k_tournament(const population_t& population, unsigned k,
                          Graph& graph,
                          std::default_random_engine& randgen) {
    auto candidatePositions = select_k_random_positions(population, k, randgen);

    const ordering_t * winner = nullptr;
    auto bestTreewidth = std::numeric_limits<unsigned>::max();
    for (auto i = 0u; i < k; ++i) {
        const auto& candidate = population[candidatePositions[i]];

        unsigned treewidth;
        graph.evaluate(candidate, &treewidth);

        if (treewidth < bestTreewidth) {
            winner = &candidate;
            bestTreewidth = treewidth;
        }
    }
    return winner;
}

}  // ns selection
}  // ns d2
