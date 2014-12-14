/**
 * Copyright 2014 Kevin Bader.
 * Distributed under the GNU GPL v3. For full terms see the file /LICENSE.
 */
#ifndef D2_DBWRITER_H
#define D2_DBWRITER_H

#include <common.h>
#include <recorder.h>
#include <unordered_map>

namespace d2 {
namespace db {

void write_validationresults(const fs::path& dbfile, const string& variant,
                             const fs::path& instance_fn,
                             const unsigned seed,
                             const float cputime_s,
                             const vector<record>& timeRecords,
                             const string& notes);

bool has_previous_validationresult(const fs::path& dbfile, const string& variant,
                                   const fs::path& instance_fn, const unsigned seed,
                                   unsigned *const treewidth = nullptr);

}  // ns db
}  // ns d2

#endif  // D2_DBWRITER_H
