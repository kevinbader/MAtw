/**
 * Copyright 2014 Kevin Bader.
 * Distributed under the GNU GPL v3. For full terms see the file /LICENSE.
 */
#include <gtest/gtest.h>
#include <common.h>
#include <population.h>

using namespace d2;

TEST(population, initializeRandomOrdering) {
    std::default_random_engine rand{0};
    const unsigned nVertices = 10;
    ordering_t ordering;
    initialize_random_ordering(rand, &ordering, nVertices);
    EXPECT_EQ(nVertices, ordering.size());
    assert_permutation(ordering, nVertices);
}

TEST(population, generateRandomPopulation) {
    std::default_random_engine rand{0};
    auto nIndividuals = 5u;
    auto nVertices = 10u;
    auto population = generate_random_population(rand, nIndividuals, nVertices);
    EXPECT_EQ(nIndividuals, population.size());
    for (const auto& individual: population) {
        EXPECT_EQ(nVertices, individual.size());
        assert_permutation(individual, nVertices);
    }
}

TEST(population, findBestIndividual) {
    auto g = Graph{7};
    g.add_edge(0, 1).add_edge(0, 2).add_edge(0, 4)
     .add_edge(1, 2)
     .add_edge(2, 3).add_edge(2, 5)
     .add_edge(4, 6)
     .add_edge(5, 6);
    population_t population;
    population.push_back(ordering_t{2, 4, 6, 3, 5, 0, 1});
    population.push_back(ordering_t{4, 6, 3, 5, 0, 1, 2});
    const ordering_t *const notReallyTheBest = &population[0];
    const ordering_t *const reallyTheBest = &population[1];

    unsigned treewidth;
    g.evaluate(*notReallyTheBest, &treewidth);
    EXPECT_EQ(4u, treewidth);

    const ordering_t * currentBest = nullptr;
    unsigned bestTreewidth;
    find_best_individual(g, population, &currentBest, &bestTreewidth);
    EXPECT_EQ(reallyTheBest, currentBest);
    EXPECT_EQ(2u, bestTreewidth);
}
