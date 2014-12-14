/**
 * Copyright 2014 Kevin Bader.
 * Distributed under the GNU GPL v3. For full terms see the file /LICENSE.
 */
#include "common.h"
#include <iomanip>  // put_time

namespace d2 {

void assert_permutation(ordering_t ordering, unsigned size) {
    assert(size <= ordering.size());
    ordering_t permutation(size);
    std::iota(permutation.begin(), permutation.end(), 0);
    std::sort(ordering.begin(), ordering.begin() + size);
    for (auto i = 0u; i < size; ++i)
        assert(permutation[i] == ordering[i]);
}

void move(ordering_t* ordering, pos_t from, pos_t to) {
    const auto nVertices = ordering->size();
    auto result = ordering_t(nVertices);

    if (to < from) {
        std::copy(ordering->cbegin(), ordering->cbegin() + to, result.begin());
        result[to] = (*ordering)[from];
        std::copy(ordering->cbegin() + to, ordering->cbegin() + from,
                  result.begin() + to + 1);
        std::copy(ordering->cbegin() + from + 1, ordering->cend(),
                  result.begin() + from + 1);
    }
    else if (to > from) {
        std::copy(ordering->cbegin() + to + 1, ordering->cend(),
                    result.begin() + to + 1);
        result[to] = (*ordering)[from];
        std::copy(ordering->cbegin() + from + 1, ordering->cbegin() + to + 1,
                  result.begin() + from);
        std::copy(ordering->cbegin(), ordering->cbegin() + from,
                  result.begin());
    }
    else {
        assert( false );
    }

    *ordering = result;
}

std::string now_as_string()
{
    //// c++11 style, not implemented by gcc 4.9
    //auto now = std::chrono::system_clock::now();
    //auto in_time_t = std::chrono::system_clock::to_time_t(now);

    //std::stringstream ss;
    //ss << std::put_time(std::localtime(&in_time_t), "%Y-%m-%d %X");
    //return ss.str();
    
    // c-style
    time_t t = time(NULL);
    tm dt;
    localtime_r(&t, &dt);
    std::stringstream ss;
    ss << (1900 + dt.tm_year)
       << std::setfill('0') << std::setw(2) << (1 + dt.tm_mon)
       << std::setfill('0') << std::setw(2) << dt.tm_mday
       << std::setfill('0') << std::setw(2) << dt.tm_hour
       << std::setfill('0') << std::setw(2) << dt.tm_min
       << std::setfill('0') << std::setw(2) << dt.tm_sec;
    return ss.str();
}

}  // ns d2

std::ostream& operator<<(std::ostream& stm, const d2::ordering_t& o) {
    stm << "[" << join(", ", o) << "]";
    return stm;
}
