/**
 * Copyright 2014 Kevin Bader.
 * Distributed under the GNU GPL v3. For full terms see the file /LICENSE.
 */
#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <common.h>
#include <mutation.h>

using namespace testing;
using namespace d2;
using namespace d2::mutation;

TEST(mutation, ism) {
     const auto orig = ordering_t{1, 2, 3, 4, 5};
     auto o = orig;
     std::default_random_engine rand;
     ISM(&o, rand);
     ASSERT_NE(orig, o);
     ASSERT_THAT(o, UnorderedElementsAre(1, 2, 3, 4, 5));
}
