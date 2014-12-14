/**
 * Copyright 2014 Kevin Bader.
 * Distributed under the GNU GPL v3. For full terms see the file /LICENSE.
 */
#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <common.h>
#include <crossover.h>
#include <population.h>

using namespace testing;
using namespace d2;
using namespace d2::crossover;

// /**
//  * Position-based crossover (POS) after Syswerda (1991).
//  */
// void POS(const ordering_t& parent1, const ordering_t& parent2,
//          ordering_t* child1, ordering_t* child2,
//          std::default_random_engine& randgen);

TEST(crossover, randomselectionbitset) {
    std::default_random_engine randgen;
    const auto nVertices = 5u;
    bitset bits = _::get_random_selection_bitset(nVertices, randgen);
    ASSERT_THAT(bits, SizeIs(nVertices));
}

TEST(crossover, selectpartnerbits) {
    std::default_random_engine randgen;
    const auto nVertices = 5u;
    for (auto i = 0u; i < 10u; ++i) {
        ordering_t ordering, partnerOrdering;
        initialize_random_ordering(randgen, &ordering, nVertices);
        initialize_random_ordering(randgen, &partnerOrdering, nVertices);
        const bitset selectionBits = _::get_random_selection_bitset(nVertices, randgen);

        bitset neededPartnerBits1
            = _::select_partner_bits_with_preprocessing(ordering, selectionBits,
                                                        partnerOrdering);
        bitset neededPartnerBits2
            = _::select_partner_bits_without_preprocessing(ordering, selectionBits,
                                                           partnerOrdering);

        ASSERT_EQ(neededPartnerBits1, neededPartnerBits2);
    }
}

TEST(crossover, pos) {
    std::default_random_engine randgen;
    const auto nVertices = 9u;
    const auto parent1 = ordering_t{1, 2, 3, 4, 5, 6, 7, 8, 9};
    const auto parent2 = ordering_t{5, 4, 6, 3, 1, 9, 2, 7, 8};
    auto selectionBits = bitset{nVertices};
    selectionBits.set(1).set(4).set(5).set(8);
    ordering_t child = _::get_child(parent1, parent2, selectionBits);
    ASSERT_THAT(child, ElementsAre(4, 2, 3, 1, 5, 6, 7, 8, 9));
}
