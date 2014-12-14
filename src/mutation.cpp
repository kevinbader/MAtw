/**
 * Copyright 2014 Kevin Bader.
 * Distributed under the GNU GPL v3. For full terms see the file /LICENSE.
 */
#include "mutation.h"

namespace d2 {
namespace mutation {

void ISM(ordering_t* ordering, std::default_random_engine& randgen) {
    const auto nVertices = ordering->size();
    auto randomVertexDist = std::uniform_int_distribution<pos_t>(0, nVertices - 1);
    pos_t fromPos = randomVertexDist(randgen);
    pos_t toPos;
    while ((toPos = randomVertexDist(randgen)) == fromPos);
    move(ordering, fromPos, toPos);
}

}  // ns mutation
}  // ns d2
