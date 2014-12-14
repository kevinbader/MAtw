/**
 * Copyright 2014 Kevin Bader.
 * Distributed under the GNU GPL v3. For full terms see the file /LICENSE.
 */
#include "localsearch.h"

namespace d2 {
namespace LS {

void minimalConflictsTreeSearch(ordering_t* ordering, Graph& graph,
                                std::default_random_engine& randgen,
                                unsigned maxNonimprovements, const clock_t maxTick,
                                unsigned* finalTreewidth) {
    unsigned treewidth;
    pos_t largestCliqueVertexPosition;
    graph.evaluate(*ordering, &treewidth, &randgen, &largestCliqueVertexPosition);
    auto bestTreewidth = treewidth;

    auto randomVertexDist = std::uniform_int_distribution<pos_t>(0, graph.nVertices() - 1);
    auto randomOtherVertex = [&randgen, &randomVertexDist, &graph](pos_t forbiddenPosition) {
        pos_t pos;
        while ((pos = randomVertexDist(randgen)) == forbiddenPosition);
        return pos;
    };

    auto nonimprovements = 0u;
    while (nonimprovements < maxNonimprovements and clock() < maxTick) {
        /// swap
        const auto fromPosition = largestCliqueVertexPosition;
        const auto toPosition = randomOtherVertex(fromPosition);
        std::swap((*ordering)[fromPosition], (*ordering)[toPosition]);
        
        graph.evaluate(*ordering, &treewidth, &randgen, &largestCliqueVertexPosition);
        if (treewidth < bestTreewidth) {
            bestTreewidth = treewidth;
            nonimprovements = 0u;
        }
        else {
            /// undo swap
            std::swap((*ordering)[fromPosition], (*ordering)[toPosition]);
            ++nonimprovements;
        }
    }
    *finalTreewidth = bestTreewidth;
}

void apply_random_movements(ordering_t* ordering, const unsigned perturbationRepetitions,
                            std::default_random_engine& randgen) {
    const auto nVertices = ordering->size();
    assert(nVertices >= 2);
    auto randomVertexDist = std::uniform_int_distribution<pos_t>(0, nVertices - 1);
    for (auto i = 0u; i < perturbationRepetitions; ++i) {
        pos_t fromPos = randomVertexDist(randgen);
        pos_t toPos;
        while ((toPos = randomVertexDist(randgen)) == fromPos);
        ::d2::move(ordering, fromPos, toPos);
    }
}

unsigned ILS(ordering_t* ordering, Graph& graph,
             std::default_random_engine& randgen,
             unsigned maxOuterNonimprovements,
             unsigned maxInnerNonimprovements,
             const clock_t maxTick) {
    auto base = *ordering;
    auto best = *ordering;

    unsigned treewidth;
    graph.evaluate(base, &treewidth, &randgen);
    auto bestTreewidth = treewidth;
    auto lastCandidateTreewidth = treewidth;

    constexpr auto degradationTolerance = 3u;
    constexpr auto minPerturbationRepetitions = 1u;
    const auto maxPerturbationRepetitions = (unsigned)std::ceil(0.05f * graph.nVertices());

    auto nonimprovements = 0u;
    auto nonchanges = 0u;        /* for perturbation-mechanism change */
    auto perturbationRepetitions = minPerturbationRepetitions;
    while (nonimprovements < maxOuterNonimprovements and clock() < maxTick) {
        ordering_t candidate = base;
        unsigned candidateTreewidth;
        // TODO rename to min conflicts tree search
        minimalConflictsTreeSearch(&candidate, graph, randgen,
                                   maxInnerNonimprovements, maxTick,
                                   &candidateTreewidth);

        if (candidateTreewidth == lastCandidateTreewidth) {
            ++nonchanges;
        }
        lastCandidateTreewidth = candidateTreewidth;

        if (candidateTreewidth < bestTreewidth) {
            /* improvement! */
            base = best = std::move(candidate);
            bestTreewidth = candidateTreewidth;
            nonimprovements = nonchanges = 0u;
            perturbationRepetitions = minPerturbationRepetitions;
        }
        else {
            ++nonimprovements;
        
            if (candidateTreewidth == bestTreewidth) {
                /* move to different, equally good solution? */
                base = best = std::move(candidate);
            }
            else if (candidateTreewidth < bestTreewidth + degradationTolerance) {
                /* reuse the slightly worse solution in the next generation */
                base = std::move(candidate);
            }
            else {
                /* forget about the candidate and go back to a better state */
                base = best;
            }
        }

        if (nonimprovements > 0 and ((float)nonchanges / (float)nonimprovements) > 0.2f) {
            /* more than 20% of the nonimprovements are also nonchanges */
            perturbationRepetitions = std::min(perturbationRepetitions + 1u,
                                               maxPerturbationRepetitions);
        }

        /* perturbate! */
        apply_random_movements(&base, perturbationRepetitions, randgen);
    }

    *ordering = std::move(best);
    return bestTreewidth;
}

}  // ns LS
}  // ns d2
