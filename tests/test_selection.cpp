/**
 * Copyright 2014 Kevin Bader.
 * Distributed under the GNU GPL v3. For full terms see the file /LICENSE.
 */
#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <common.h>
#include <selection.h>

using namespace testing;
using namespace d2;
using namespace d2::selection;

/** assertion death tests **/
#ifndef NDEBUG
TEST(selectionDeathTest, ktournamentZerok) {
    auto population = population_t{};
    auto g = Graph{1};
    auto randgen = std::default_random_engine{};
    ASSERT_DEATH(k_tournament(population, 0, g, randgen), "Assertion .* failed\\.");
}

TEST(selectionDeathTest, ktournamentToohighk) {
    auto population = population_t{};
    auto g = Graph{1};
    auto randgen = std::default_random_engine{};
    ASSERT_DEATH(k_tournament(population, 1, g, randgen), "Assertion .* failed\\.");
}
#endif  // NDEBUG

// TODO more tests with mocked random engine
