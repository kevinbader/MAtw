/**
 * Copyright 2014 Kevin Bader.
 * Distributed under the GNU GPL v3. For full terms see the file /LICENSE.
 */
#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <common.h>
#include <dbwriter.h>

using namespace testing;
using namespace d2;
using namespace db;

void setup(fs::path *const dbfile) {
    // TODO make new file with random suffix
    *dbfile = fs::path{"/tmp/test_dbwriter_1234.db"};
}

void teardown(const fs::path& dbfile) {
    fs::remove(dbfile);
}

TEST(dbwriter, write) {
    fs::path dbfile;
    setup(&dbfile);
    const auto thevariant = string{"TEST"};
    const auto instance = string{"testinstance.col"};
    const auto seed = 0u;
    const auto notes = string{"some notes"};
    auto recorder = Recorder{};
    const auto nRows = 100000u;
    for (auto i = 0u; i < nRows; ++i) {
        recorder.add(i * 1000u / nRows * CLOCKS_PER_SEC, rand() % 200);
    }
    const auto cputime_s = 9.99f;
    write_validationresults(dbfile, thevariant, instance, seed, cputime_s,
                            *recorder.data(), notes);
    teardown(dbfile);
}
