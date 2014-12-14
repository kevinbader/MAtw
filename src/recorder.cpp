/**
 * Copyright 2014 Kevin Bader.
 * Distributed under the GNU GPL v3. For full terms see the file /LICENSE.
 */
#include "recorder.h"

namespace d2 {

void Recorder::print(std::ostream& oss) const {
    if (_records.size() == 0)
        return;
    const auto offset = std::get<1>(_records[0]);
    for (const auto& record: _records) {
        const auto second = (float)(std::get<1>(record) - offset) / CLOCKS_PER_SEC;
        const auto treewidth = std::get<2>(record);
        if (std::get<0>(record) == RecordType::IMPROVEMENT)
            oss << "[improvement] ";
        else
            oss << "              ";
        oss << second << "\t" << treewidth << endl;
    }
}

}  // ns d2
