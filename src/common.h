/**
 * Copyright 2014 Kevin Bader.
 * Distributed under the GNU GPL v3. For full terms see the file /LICENSE.
 */
#ifndef D2_COMMON_H
#define D2_COMMON_H

#include <algorithm>
#include <array>
#include <boost/dynamic_bitset.hpp>
#include <boost/filesystem.hpp>
#include <cassert>
#include <chrono>
#include <iostream>
#include <string>
#include <sstream>
#include <vector>

namespace d2 {

using std::array;
using std::cerr;
using std::clog;
using std::cout;
using std::endl;
using std::string;
using bitset = ::boost::dynamic_bitset<unsigned long, std::allocator<unsigned long>>;
namespace chrono = std::chrono;
namespace fs = ::boost::filesystem;

/* use vector<char> instead of the bitset-like special implementation for vector<bool> */
template<typename t>
struct vectortypeproxy {
  using type = t;
};
template<>
struct vectortypeproxy<bool> {
  using type = char;
};
template<typename t, typename... p>
using vector = std::vector<typename vectortypeproxy<t>::type, p...>;

using vertex_t = unsigned;
using pos_t = unsigned;
using ordering_t = vector<vertex_t>;
using population_t = vector<ordering_t>;

void assert_permutation(ordering_t ordering, unsigned size);

void move(ordering_t* ordering, pos_t from, pos_t to);

string now_as_string();

}  // ns d2

template<class T>
std::string join(const std::string& separator, const std::vector<T>& v) {
    std::stringstream ss;
    for (auto it = v.cbegin(); it != v.cend(); ++it) {
        if (it != v.begin()) {
            ss << separator;
        }
        ss << *it;
    }
    return ss.str();
}

std::ostream& operator<<(std::ostream& stm, const d2::ordering_t& o);

#endif  // D2_COMMON_H

