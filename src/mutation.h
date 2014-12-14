/**
 * Copyright 2014 Kevin Bader.
 * Distributed under the GNU GPL v3. For full terms see the file /LICENSE.
 */
#ifndef D2_MUTATION_H
#define D2_MUTATION_H

#include <random>
#include <common.h>
#include <graph.h>

namespace d2 {
namespace mutation {

/**
 * ISM = Insertion Mutation Operator.
 *
 * Randomly picks an element and _moves_ it to a random location within the ordering.
 */
void ISM(ordering_t* ordering, std::default_random_engine& randgen);

}  // ns mutation
}  // ns d2

#endif  // D2_MUTATION_H
