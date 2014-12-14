/**
 * Copyright 2014 Kevin Bader.
 * Distributed under the GNU GPL v3. For full terms see the file /LICENSE.
 */
#ifndef D2_CONFIG_H
#define D2_CONFIG_H

#define MA3          1
#define MA3_         2
#define MA2           4
#define MA2_          8
#define MA1          16
#define MA1_         32
#ifndef VARIANT
static_assert(false, "Please define VARIANT; see config.h for possible values."
              " Example: -DVARIANT=(MA3|MA3_) for the default implementation"
              " of MA3");
#else
static_assert((VARIANT & MA3) or
              (VARIANT & MA2) or
              (VARIANT & MA1),
              "unknown VARIANT; see config.h");
static_assert(not (VARIANT & MA3) or ((VARIANT & MA3_) /* XOR MA3_OTHER */),
              "Please set both, MA3 and MA3_*");
static_assert(not (VARIANT & MA2) or ((VARIANT & MA2_) /* XOR MA2_OTHER */),
              "Please set both, MA2 and MA2_*");
static_assert(not (VARIANT & MA1) or ((VARIANT & MA1_) /* XOR MA1_OTHER */),
              "Please set both, MA1 and MA1_*");
#endif
#define IS_VARIANT(a) (VARIANT & (a))

#include <common.h>
#include <memory>
#include <chrono>
#include <unordered_map>

namespace d2 {

constexpr const char* variant() {
#if IS_VARIANT(MA3_)
    return "MA3_";
#elif IS_VARIANT(MA2_)
    return "MA2_";
#elif IS_VARIANT(MA1_)
    return "MA1_";
#endif
}

string variantname();

class ConfigParseException : public std::logic_error {
public:
    ConfigParseException(const string& msg) throw() : std::logic_error(msg) {}
    virtual ~ConfigParseException() throw() {}
};

class OutOfRangeException : public ConfigParseException {
public:
    OutOfRangeException(const string& msg) throw() : ConfigParseException(msg) {}
    virtual ~OutOfRangeException() throw() {}
};

#if IS_VARIANT(MA3)
    struct Config {
        bool doDotOutputOnly;
        fs::path dbfile;
        bool nodb;
        unsigned seed;
        std::chrono::steady_clock::duration runningTime;
        fs::path instance_fn;
        unsigned populationSize;
        const float mutationProbability = 0.3f;
        const unsigned tournamentSize = 3u;
        unsigned maxIlsOuterNonimprovements;
        const unsigned maxIlsInnerNonimprovements = 10u;
        float localsearchFraction;

        /**
         * NOTE: things to do when adding a parameter!
         * - write it down here
         * - add it to an option group
         * - add it to to_string
         * - add it to to_map (if applicable)
         * - add it to operator== (if applicable)
         * - add it to the conversion if-else in matches_parameters (if applicable)
         */

        std::unordered_map<string, string> to_map() const;
        string to_string() const;
        string to_compact_string() const;
        bool operator== (const Config& other) const;
    };
#elif IS_VARIANT(MA2)
    struct Config {
        bool doDotOutputOnly;
        fs::path dbfile;
        bool nodb;
        unsigned seed;
        std::chrono::steady_clock::duration runningTime;
        fs::path instance_fn;
        unsigned populationSize;
        float crossoverProbability;
        unsigned tournamentSize;
        unsigned ilsMaxOuterNonimprovements;
        unsigned ilsMaxInnerNonimprovements;
        float localsearchFraction;

        /**
         * NOTE: things to do when adding a parameter!
         * - write it down here
         * - add it to an option group
         * - add it to to_string
         * - add it to to_map (if applicable)
         * - add it to operator== (if applicable)
         * - add it to the conversion if-else in matches_parameters (if applicable)
         */

        std::unordered_map<string, string> to_map() const;
        string to_string() const;
        string to_compact_string() const;
        bool operator== (const Config& other) const;
    };
#elif IS_VARIANT(MA1)
    struct Config {
        bool doDotOutputOnly;
        fs::path dbfile;
        bool nodb;
        unsigned seed;
        std::chrono::steady_clock::duration runningTime;
        fs::path instance_fn;
        unsigned populationSize;
        float partnerFraction;
        float opponentFraction;
        unsigned ilsMaxOuterNonimprovements;
        unsigned ilsMaxInnerNonimprovements;

        /**
         * NOTE: things to do when adding a parameter!
         * - write it down here
         * - add it to an option group
         * - add it to to_string
         * - add it to to_map (if applicable)
         * - add it to operator== (if applicable)
         * - add it to the conversion if-else in matches_parameters (if applicable)
         */

        std::unordered_map<string, string> to_map() const;
        string to_string() const;
        string to_compact_string() const;
        bool operator== (const Config& other) const;
    };
#endif  // VARIANTMAJOR

using Config_t = std::shared_ptr<const Config>;
Config_t parse(const int argc, const char* argv[]);

template<class T, class R, class S>
std::out_of_range _make_out_of_range_error(T what, R expected, S actual) {
    std::stringstream ss;
    ss << "Out of range for '" << what
       << "': expected " << expected << ", got " << actual;
    return std::out_of_range(ss.str());
}

}  // d2


#endif  // D2_CONFIG_H
