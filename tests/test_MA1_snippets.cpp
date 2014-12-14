/**
 * Copyright 2014 Kevin Bader.
 * Distributed under the GNU GPL v3. For full terms see the file /LICENSE.
 */
#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <common.h>

using namespace testing;
using namespace d2;

void test_opponent_selection(const vector<unsigned>&& v, const unsigned forIndex,
                             const unsigned nOpponents, const vector<unsigned>&& expected) {
    const auto populationSize = v.size();
    auto result = [&]{
        unsigned startIndex;
        if (nOpponents % 2 == 0) {
            if (forIndex >= nOpponents / 2) {
                startIndex = forIndex - nOpponents / 2;
            }
            else {
                startIndex = populationSize - (nOpponents / 2 - forIndex);
            }
        }
        else {
            if (forIndex >= nOpponents / 2) {
                startIndex = forIndex - nOpponents / 2;
            }
            else {
                startIndex = populationSize - (nOpponents / 2 - forIndex);
            }
        }
        auto result = vector<unsigned>();
        const auto forIndexTmp = startIndex < forIndex ? forIndex : forIndex + populationSize;
        for (auto i = 0u; i < nOpponents; ++i) {
            // random here (i = ...)
            unsigned opponentIndex;
            if (startIndex + i < forIndexTmp) {
                opponentIndex = (startIndex + i) % populationSize;
            }
            else {
                opponentIndex = (startIndex + i + 1) % populationSize;
            }
            result.push_back(opponentIndex);
        }
        return result;
    }();
    ASSERT_THAT(result, UnorderedElementsAreArray(expected));
}

void test_partner_selection(const vector<unsigned>&& v, const unsigned forIndex,
                            const unsigned nPartners, const vector<unsigned>&& expected) {
    const auto nPopulation = v.size();
    auto result = [&]{
        const auto flooredMidNomod = forIndex + nPopulation / 2;
        unsigned startIndex;
        if (nPopulation % 2 == 0) {
            startIndex = (flooredMidNomod - (nPartners - 1) / 2) % nPopulation;
        }
        else {
            startIndex = (flooredMidNomod - (nPartners - 2) / 2) % nPopulation;
        }
        auto result = vector<unsigned>();
        for (auto i = 0u; i < nPartners; ++i) {
            // random here (i = ...)
            auto partnerIndex = (startIndex + i) % nPopulation;
            result.push_back(partnerIndex);
        }
        return result;
    }();
    ASSERT_THAT(result, UnorderedElementsAreArray(expected));
}

TEST(MA1, opponentselection) {
    test_opponent_selection(vector<unsigned>(6), 0, 4, {4, 5, 1, 2});
    test_opponent_selection(vector<unsigned>(6), 3, 4, {1, 2, 4, 5});
    test_opponent_selection(vector<unsigned>(6), 5, 4, {3, 4, 0, 1});
    test_opponent_selection(vector<unsigned>(6), 0, 3, {5, 1, 2});
    test_opponent_selection(vector<unsigned>(6), 3, 3, {2, 4, 5});
    test_opponent_selection(vector<unsigned>(6), 5, 3, {4, 0, 1});
    test_opponent_selection(vector<unsigned>(7), 0, 4, {5, 6, 1, 2});
    test_opponent_selection(vector<unsigned>(7), 3, 4, {1, 2, 4, 5});
    test_opponent_selection(vector<unsigned>(7), 6, 4, {4, 5, 0, 1});
    test_opponent_selection(vector<unsigned>(7), 0, 3, {6, 1, 2});
    test_opponent_selection(vector<unsigned>(7), 3, 3, {2, 4, 5});
    test_opponent_selection(vector<unsigned>(7), 6, 3, {5, 0, 1});
}

TEST(MA1, partnerselection) {
    test_partner_selection(vector<unsigned>(6), 0, 4, {2, 3, 4, 5});
    test_partner_selection(vector<unsigned>(6), 2, 4, {4, 5, 0, 1});
    test_partner_selection(vector<unsigned>(6), 5, 4, {1, 2, 3, 4});
    test_partner_selection(vector<unsigned>(6), 0, 3, {2, 3, 4});
    test_partner_selection(vector<unsigned>(6), 2, 3, {4, 5, 0});
    test_partner_selection(vector<unsigned>(6), 5, 3, {1, 2, 3});
    test_partner_selection(vector<unsigned>(7), 0, 4, {2, 3, 4, 5});
    test_partner_selection(vector<unsigned>(7), 3, 4, {5, 6, 0, 1});
    test_partner_selection(vector<unsigned>(7), 6, 4, {1, 2, 3, 4});
    test_partner_selection(vector<unsigned>(7), 0, 3, {3, 4, 5});
    test_partner_selection(vector<unsigned>(7), 3, 3, {6, 0, 1});
    test_partner_selection(vector<unsigned>(7), 6, 3, {2, 3, 4});
}
