/**
 * Copyright 2014 Kevin Bader.
 * Distributed under the GNU GPL v3. For full terms see the file /LICENSE.
 */
#include "crossover.h"
#include <unordered_map>

namespace d2 {
namespace crossover {
using namespace _;

bitset _::get_random_selection_bitset(const size_t nVertices,
                                      std::default_random_engine& randgen) {
    auto selectionBits = bitset{nVertices};
    const auto nSelectionBitsBlocks = selectionBits.num_blocks();
    assert(nSelectionBitsBlocks == ceil(nVertices / float(8 * sizeof(unsigned long))));
    auto dist = std::uniform_int_distribution<unsigned long>{0,
                                                std::numeric_limits<unsigned long>::max()};
    while (selectionBits.none()) {
        vector<unsigned long> selectionLongs(nSelectionBitsBlocks);
        for (auto i = 0u; i < nSelectionBitsBlocks; ++i ) {
            selectionLongs[i] = dist(randgen);
        }
        boost::from_block_range(selectionLongs.cbegin(), selectionLongs.cend(),
                                selectionBits );
        /* clear bits with index >= nVertices */
        for (auto i = nVertices;
             i < selectionBits.bits_per_block * nSelectionBitsBlocks; ++i) {
            selectionBits[i] = false;
        }
    }
    return selectionBits;
}

bitset _::select_partner_bits_with_preprocessing(const ordering_t& ordering,
                                               const bitset& selectionBits,
                                               const ordering_t& partnerOrdering) {
    const auto nVertices = ordering.size();
    assert(nVertices == partnerOrdering.size());
    assert(nVertices == selectionBits.size());
    auto neededPartnerBits = bitset(nVertices);
    neededPartnerBits.flip();

    using vertexId_to_position_hashmap = std::unordered_map<vertex_t, pos_t>;
    auto posMap = vertexId_to_position_hashmap(partnerOrdering.size());
    for (auto i = 0u; i < nVertices; ++i) {
        posMap[partnerOrdering[i]] = i;
    }
    for (size_t pos = selectionBits.find_first(); pos != bitset::npos;
         pos = selectionBits.find_next(pos)) {
        neededPartnerBits[posMap[ordering[pos]]] = false;
    }

    return neededPartnerBits;
}

bitset _::select_partner_bits_without_preprocessing(const ordering_t& ordering,
                                                  const bitset& selectionBits,
                                                  const ordering_t& partnerOrdering) {
    const auto nVertices = ordering.size();
    assert(nVertices == partnerOrdering.size());
    assert(nVertices == selectionBits.size());
    auto neededPartnerBits = bitset(nVertices);
    neededPartnerBits.flip();

    for (size_t pos = selectionBits.find_first(); pos != bitset::npos;
         pos = selectionBits.find_next(pos)) {
        const auto vertex = ordering[pos];
        for (auto i = 0u; i < nVertices; ++i) {
            if (partnerOrdering[i] == vertex) {
                neededPartnerBits.set( i, false );
                break; // search for this vertex
            }
        }
    }

    return neededPartnerBits;
}

ordering_t _::get_child(const ordering_t& parent1, const ordering_t& parent2,
                        const bitset& selectionBits) {
    const auto nVertices = parent1.size();
    auto result = ordering_t(nVertices);

    for (auto pos = selectionBits.find_first(); pos != bitset::npos;
         pos = selectionBits.find_next(pos)) {
        result[pos] = parent1[pos];
    }

    /* "remove" vertices that are already present in the result */
    /* "remove" unneeded partner bits
     * experiments suggest that building a map of the positions pays off
     * when nVertices > 500 */
    bitset neededPartnerBits;
    if(nVertices > 500) {
        auto tmp = select_partner_bits_with_preprocessing(result, selectionBits, parent2);
        neededPartnerBits = std::move(tmp);
    }
    else {
        auto tmp = select_partner_bits_without_preprocessing(result, selectionBits, parent2);
        neededPartnerBits = std::move(tmp);
    }

    /* adopt remaining vertices from the partner's ordering in their order */
    for (size_t i = 0u, pos = neededPartnerBits.find_first();
         i < nVertices && pos != bitset::npos; ++i) {
        if(not selectionBits[i]) {
            result[i] = parent2[pos];
            pos = neededPartnerBits.find_next( pos );
        }
    }

    return result;
}

void POS(const ordering_t& parent1, const ordering_t& parent2,
         ordering_t* child1, ordering_t* child2,
         std::default_random_engine& randgen) {
    if (parent1 == parent2) {
        *child1 = *child2 = parent1;
    }
    else {
        const auto nVertices = parent1.size();
        assert(nVertices == parent2.size());
        bitset selectionBits = get_random_selection_bitset(nVertices, randgen);
        *child1 = get_child(parent1, parent2, selectionBits);
        *child2 = get_child(parent2, parent1, selectionBits);
    }
}

void POS_single(const ordering_t& parent1, const ordering_t& parent2,
                ordering_t* child,
                std::default_random_engine& randgen) {
    if (parent1 == parent2) {
        *child = parent1;
    }
    else {
        const auto nVertices = parent1.size();
        assert(nVertices == parent2.size());
        bitset selectionBits = get_random_selection_bitset(nVertices, randgen);
        *child = get_child(parent1, parent2, selectionBits);
    }
}

}  // ns crossover
}  // ns d2
