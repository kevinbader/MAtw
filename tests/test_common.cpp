/**
 * Copyright 2014 Kevin Bader.
 * Distributed under the GNU GPL v3. For full terms see the file /LICENSE.
 */
#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <common.h>

using namespace testing;
using namespace d2;

TEST(common, moveTohead) {
     auto o = ordering_t{1, 2, 3, 4, 5};
     move(&o, 2, 0);
     ASSERT_THAT(o, ElementsAre(3, 1, 2, 4, 5));
}

TEST(common, moveHeadbound) {
     auto o = ordering_t{1, 2, 3, 4, 5};
     move(&o, 2, 1);
     ASSERT_THAT(o, ElementsAre(1, 3, 2, 4, 5));
}

TEST(common, moveTailbound) {
     auto o = ordering_t{1, 2, 3, 4, 5};
     move(&o, 2, 3);
     ASSERT_THAT(o, ElementsAre(1, 2, 4, 3, 5));
}

TEST(common, moveTotail) {
     auto o = ordering_t{1, 2, 3, 4, 5};
     move(&o, 2, 4);
     ASSERT_THAT(o, ElementsAre(1, 2, 4, 5, 3));
}

TEST(common, moveFromheadtomiddle) {
     auto o = ordering_t{1, 2, 3, 4, 5};
     move(&o, 0, 2);
     ASSERT_THAT(o, ElementsAre(2, 3, 1, 4, 5));
}

TEST(common, moveFromheadtotail) {
     auto o = ordering_t{1, 2, 3, 4, 5};
     move(&o, 0, 4);
     ASSERT_THAT(o, ElementsAre(2, 3, 4, 5, 1));
}

TEST(common, moveFromtailtomiddle) {
     auto o = ordering_t{1, 2, 3, 4, 5};
     move(&o, 4, 2);
     ASSERT_THAT(o, ElementsAre(1, 2, 5, 3, 4));
}

TEST(common, moveFromtailtohead) {
     auto o = ordering_t{1, 2, 3, 4, 5};
     move(&o, 4, 0);
     ASSERT_THAT(o, ElementsAre(5, 1, 2, 3, 4));
}

/** assertion death tests **/
#ifndef NDEBUG
TEST(commonDeathTest, moveTosame) {
     auto o = ordering_t{1, 2, 3, 4, 5};
     ASSERT_DEATH(move(&o, 2, 2), "Assertion .* failed\\.");
}
#endif
