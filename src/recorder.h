/**
 * Copyright 2014 Kevin Bader.
 * Distributed under the GNU GPL v3. For full terms see the file /LICENSE.
 */
#ifndef D2_RECORDER_H
#define D2_RECORDER_H

#include <ctime>
#include <common.h>

namespace d2 {

enum class RecordType {
    ITERATION,
    IMPROVEMENT
};
constexpr const char* name_record_type(const RecordType& type) {
    return (type == RecordType::ITERATION) ? "iteration" : "improvement";
}

namespace Record {
    enum RecordEnum {
        TYPE = 0,
        TICK = 1,
        TREEWIDTH = 2
    };
}  // ns Record
using record = std::tuple<RecordType, clock_t, unsigned>;

class Recorder {
public:
    inline
    void add(clock_t tick, unsigned treewidth, RecordType type = RecordType::ITERATION) {
        _records.push_back(record{type, tick, treewidth});
    }

    void print(std::ostream& oss) const;

    vector<record>* data() {
        return &_records;
    }
private:
    vector<record> _records;
};

}  // ns d2

#endif  // D2_RECORDER_H
