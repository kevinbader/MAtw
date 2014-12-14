/**
 * Copyright 2014 Kevin Bader.
 * Distributed under the GNU GPL v3. For full terms see the file /LICENSE.
 */
#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <common.h>
#include <population.h>
#include "graph.h"

using namespace d2;
using namespace testing;

TEST(graph, initialization) {
    const auto nVertices = 10u;
    Graph g{nVertices};
    for (auto i = 0u; i < nVertices; ++i) {
        for (auto j = 0u; j < nVertices; ++j) {
            EXPECT_FALSE(g.matrix(i, j));
        }
        EXPECT_EQ(0u, g.neighborlistLengths(i));
    }
}

TEST(graph, addingEdges) {
    Graph g{2};
    EXPECT_FALSE(g.matrix(0, 1));
    EXPECT_FALSE(g.matrix(1, 0));
    EXPECT_EQ(0u, g.neighborlistLengths(0));
    EXPECT_EQ(0u, g.neighborlistLengths(1));
    g.add_edge(0, 1);
    EXPECT_TRUE(g.matrix(0, 1));
    EXPECT_TRUE(g.matrix(1, 0));
    ASSERT_EQ(1u, g.neighborlistLengths(0));
    ASSERT_EQ(1u, g.neighborlistLengths(1));
    EXPECT_EQ(1u, g.neighborlists(0, 0));
    EXPECT_EQ(0u, g.neighborlists(1, 0));
}

bool has_matrix_equal_to(const Graph& g, const vector<vector<bool>>& m, unsigned nVertices) {
    for (auto i = 0u; i < nVertices; ++i)
        for (auto j = 0u; j < nVertices; ++j)
            if (g.matrix(i, j) != m[i][j])
                return false;
    return true;
}

bool has_neighborlistlengths_equal_to(const Graph& g, vector<size_t>& l, unsigned nVertices) {
    for (auto i = 0u; i < nVertices; ++i)
        if (g.neighborlistLengths(i) != l[i])
            return false;
    return true;
}

TEST(graph, evaluateInvariant) {
    const unsigned nVertices = 4;
    Graph g{nVertices};
    g.add_edge(0, 1);
    g.add_edge(0, 2);
    auto matrixcopy = vector<vector<bool>>();
    auto listscopy = vector<size_t>(nVertices);
    for (auto i = 0u; i < nVertices; ++i) {
        matrixcopy.emplace_back(nVertices);
        for (auto j = 0u; j < nVertices; ++j)
            matrixcopy[i][j] = g.matrix(i, j);
        listscopy[i] = g.neighborlistLengths(i);
    }
        
    std::default_random_engine randgen;
    ordering_t ordering;
    initialize_random_ordering(randgen, &ordering, nVertices);
    unsigned treewidth;
    g.evaluate(ordering, &treewidth);
    
    EXPECT_TRUE(has_matrix_equal_to(g, matrixcopy, nVertices));
    EXPECT_TRUE(has_neighborlistlengths_equal_to(g, listscopy, nVertices));
    g.add_edge(0, 3);
    EXPECT_FALSE(has_matrix_equal_to(g, matrixcopy, nVertices));
    EXPECT_FALSE(has_neighborlistlengths_equal_to(g, listscopy, nVertices));
}

TEST(graph, largestcliqueEx1Tw4) {
    auto g = Graph{7};
    g.add_edge(0, 1).add_edge(0, 2).add_edge(0, 4)
     .add_edge(1, 2)
     .add_edge(2, 3).add_edge(2, 5)
     .add_edge(4, 6)
     .add_edge(5, 6);
    const auto ordering = ordering_t{2, 4, 6, 3, 5, 0, 1};

    std::default_random_engine randgen;
    unsigned treewidth;
    pos_t largestCliqueVertexPosition;
    vector<pos_t> largestCliqueVertexPositions;
    g.evaluate(ordering, &treewidth, &randgen,
               &largestCliqueVertexPosition, &largestCliqueVertexPositions);
    EXPECT_EQ(4u, treewidth);
    ASSERT_THAT(largestCliqueVertexPosition, AnyOf(0u, 3u, 4u, 5u, 6u));
    ASSERT_THAT(largestCliqueVertexPositions, UnorderedElementsAre(0u, 3u, 4u, 5u, 6u));
}

TEST(graph, largestcliqueEx1Tw2) {
    auto g = Graph{7};
    g.add_edge(0, 1).add_edge(0, 2).add_edge(0, 4)
     .add_edge(1, 2)
     .add_edge(2, 3).add_edge(2, 5)
     .add_edge(4, 6)
     .add_edge(5, 6);
    const auto ordering = ordering_t{4, 6, 3, 5, 0, 1, 2};

    std::default_random_engine randgen;
    unsigned treewidth;
    pos_t largestCliqueVertexPosition;
    vector<pos_t> largestCliqueVertexPositions;
    g.evaluate(ordering, &treewidth, &randgen,
               &largestCliqueVertexPosition, &largestCliqueVertexPositions);
    EXPECT_EQ(2u, treewidth);
    ASSERT_THAT(largestCliqueVertexPositions, Contains(largestCliqueVertexPosition));
}
