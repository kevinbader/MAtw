/**
 * Copyright 2014 Kevin Bader.
 * Distributed under the GNU GPL v3. For full terms see the file /LICENSE.
 */
#ifndef D2_CROSSOVER_H
#define D2_CROSSOVER_H

#include <random>
#include <common.h>
#include <graph.h>

namespace d2 {
namespace crossover {

/**
 * Position-based crossover (POS) after Syswerda (1991).
 */
void POS(const ordering_t& parent1, const ordering_t& parent2,
         ordering_t* child1, ordering_t* child2,
         std::default_random_engine& randgen);

void POS_single(const ordering_t& parent1, const ordering_t& parent2,
                ordering_t* child,
                std::default_random_engine& randgen);

/**
 * private methods, in header file for use in tests
 */
namespace _ {
/**
 * creates a bitset that contains nVertices randomly-set bits.
 */
bitset get_random_selection_bitset(const size_t nVertices,
                                   std::default_random_engine& randgen);

bitset select_partner_bits_with_preprocessing(const ordering_t& ordering,
                                              const bitset& selectionBits,
                                              const ordering_t& partnerOrdering);

bitset select_partner_bits_without_preprocessing(const ordering_t& ordering,
                                                 const bitset& selectionBits,
                                                 const ordering_t& partnerOrdering);

ordering_t get_child(const ordering_t& parent1, const ordering_t& parent2,
                     const bitset& selectionBits);
}  // ns _

}  // ns crossover
}  // ns d2

#endif  // D2_CROSSOVER_H
