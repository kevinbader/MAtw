/**
 * Copyright 2014 Kevin Bader.
 * Distributed under the GNU GPL v3. For full terms see the file /LICENSE.
 */
#include "dbwriter.h"
#include <thread>

extern "C" {
#include <sqlite3.h>
}

namespace d2 {
namespace db {


class DbException : public std::runtime_error {
public:
    explicit DbException (const string& what_arg)
        : std::runtime_error(what_arg) {}
    explicit DbException (const char* what_arg)
        : std::runtime_error(what_arg) {}
};

class Db {
public:
    Db(const fs::path& dbfile)
        : _db(nullptr), _dbfile(dbfile)
    {}
    ~Db() {
        try {
            close();
        }
        catch (const DbException& ex) {
            cerr << ex.what() << endl;
        }
    }
    void open() {
        //cout << "opening sqlite db " << _dbfile << endl;
        int ret = sqlite3_open(_dbfile.c_str(), &_db);
        if (ret != SQLITE_OK) {
            std::stringstream ss;
            ss << "could not open sqlite db at " << _dbfile << ": "
               << sqlite3_errstr(ret) << " (code " << ret << ")";
            throw DbException(ss.str());
        }
    }
    void close() {
        //cout << "closing sqlite db " << _dbfile << endl;
        int ret = sqlite3_close(_db);
        if (ret != SQLITE_OK) {
            std::stringstream ss;
            ss << "error while closing " << _dbfile << ": "
               << sqlite3_errstr(ret) << " (code " << ret << ")";
            throw DbException(ss.str());
        }
        _db = nullptr;
    }
    sqlite3* operator*() {
        return _db;
    }
private:
    sqlite3 * _db;
    const fs::path _dbfile;
};

class Stmt {
public:
    Stmt(Db& db)
        : _db(db)
    {
    }
    ~Stmt() {
        //cout << "destroying sqlite statement " << endl;
        (void) sqlite3_finalize(_stmt);
    }
    Stmt& operator()(const char *const sql, bool doStep=true)
    {
        _sql = sql;
        //cout << "SQL: '" << sql << "' -> ";
        _status = sqlite3_prepare_v2(*_db, sql, -1, &_stmt, nullptr);
        if (_status != SQLITE_OK) {
            //cout << "FAIL" << endl;
            std::stringstream ss;
            ss << "could not prepare statement '" << sql << "': "
               << sqlite3_errstr(_status) << " (code " << _status << ")";
            throw DbException(ss.str());
        }
        if (doStep)
            step();
        //cout << "OK" << endl;
        return *this;
    }
    void reset() {
        _status = sqlite3_reset(_stmt);
        if (_status != SQLITE_OK) {
            std::stringstream ss;
            ss << "could not reset statement '" << _sql << "': "
               << sqlite3_errstr(_status) << " (code " << _status << ")";
            throw DbException(ss.str());
        }
        _bindIndex = 1;
    }
    Stmt& operator<<(const char* text) {
        _status = sqlite3_bind_text(_stmt, _bindIndex, text, -1, SQLITE_TRANSIENT);
        if (_status != SQLITE_OK) {
            std::stringstream ss;
            ss << "could not bind (" << _bindIndex << ", " << text
               << ") to statement '" << _sql << "': "
               << sqlite3_errstr(_status) << " (code " << _status << ")";
            throw DbException(ss.str());
        }
        ++_bindIndex;
        return *this;
    }
    Stmt& operator<<(unsigned u) {
        return operator<<(std::to_string(u).c_str());
    }
    Stmt& operator<<(float f) {
        return operator<<(std::to_string(f).c_str());
    }
    Stmt& operator<<(const string& str) {
        return operator<<(str.c_str());
    }
    void step() {
        _status = sqlite3_step(_stmt);
        if (_status != SQLITE_DONE and _status != SQLITE_ROW) {
            std::stringstream ss;
            ss << "error while executing statement '" << _sql << "': "
               << sqlite3_errstr(_status) << " (code " << _status << ")";
            throw DbException(ss.str());
        }
    }
    sqlite3_stmt* operator*() {
        return _stmt;
    }
    bool has_next_row() {
        return _status == SQLITE_ROW;
    }
private:
    Db& _db;
    sqlite3_stmt * _stmt = nullptr;
    int _status = SQLITE_MISUSE;
    string _sql;
    int _bindIndex = 1;
};

void sleep_a_bit() {
    std::this_thread::sleep_for(std::chrono::milliseconds(500) + std::chrono::milliseconds(rand() % 1000));
}

void write_validationresults(const fs::path& dbfile, const string& variant,
                             const fs::path& instance_fn,
                             const unsigned seed,
                             const float cputime_s,
                             const vector<record>& timeRecords,
                             const string& notes) {
    string cols, vals;
    auto db = Db{dbfile};
    try {
        db.open();
        Stmt{db}("PRAGMA foreign_keys = ON");
        Stmt{db}("PRAGMA busy_timeout = 60000");
    }
    catch (const DbException& ex) {
        cerr << "ERROR: " << ex.what() << endl;
        return;
    }

    const auto nRetries = 10u;
    auto retryNo = 0u;
    while (retryNo++ < nRetries) {
        try {
            Stmt{db}("BEGIN;");
            std::stringstream ssql;
            ssql << "CREATE TABLE IF NOT EXISTS validationresults ("
                    " 'variant' TEXT NOT NULL,"
                    " 'instance' TEXT NOT NULL,"
                    " 'seed' INTEGER NOT NULL,"
                    " 'runtime_s' REAL NOT NULL,"
                    " 'treewidth' INTEGER NOT NULL,"
                    " 'totalruntime_s' REAL NOT NULL,"
                    " 'notes' TEXT)";
            Stmt{db}(ssql.str().c_str());
            Stmt{db}("COMMIT;");
            break;
        }
        catch (const DbException& ex) {
            cerr << "WARN: " << ex.what() << "; retrying in 1 second" << endl;
            sleep_a_bit();
        }
    }

    retryNo = 0u;
    while (retryNo++ < nRetries) {
        try {
            Stmt{db}("BEGIN;");
            const auto nRecords = timeRecords.size();
            if (nRecords > 0u) {
                std::stringstream ssql;
                ssql << "INSERT INTO validationresults VALUES (?, ?, ?, ?, ?, ?, ?)";
                auto stmt = Stmt{db};
                stmt(ssql.str().c_str(), false);
                for (auto i = 0u; i < nRecords; ++i) {
                    if (std::get<Record::TYPE>(timeRecords[i]) != RecordType::IMPROVEMENT) {
                        continue;
                    }
                    auto treewidth = std::get<Record::TREEWIDTH>(timeRecords[i]);
                    const auto second
                        = (float)std::get<Record::TICK>(timeRecords[i]) / CLOCKS_PER_SEC;
                    stmt.reset();
                    stmt << variant << instance_fn.stem().native() << seed
                        << second << treewidth << cputime_s << notes;
                    stmt.step();
                }
            }
            Stmt{db}("COMMIT;");
            break;
        }
        catch (const DbException& ex) {
            cerr << "WARN: " << ex.what() << "; retrying in 1 second" << endl;
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }
    }
}

bool has_previous_validationresult(const fs::path& dbfile, const string& variant,
                                   const fs::path& instance_fn, const unsigned seed,
                                   unsigned *const treewidth) {
    if (not fs::exists(dbfile)) {
        return false;
    }
    auto db = Db{dbfile};
    try {
        db.open();
        Stmt{db}("PRAGMA busy_timeout = 60000");
    }
    catch (const DbException& ex) {
        cerr << "WARN: " << ex.what() << "; retrying in 1 second" << endl;
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }

    const auto nRetries = 10u;
    auto retryNo = 0u;
    while (retryNo++ < nRetries) {
        try {
            std::stringstream ssql;
            ssql << "SELECT min(treewidth) FROM validationresults WHERE"
                    " variant=? and instance=? and seed=?";
            auto stmt = Stmt{db};
            stmt(ssql.str().c_str(), false);
            stmt << variant << instance_fn.stem().native() << seed;
            stmt.step();
            if (stmt.has_next_row()) {
                assert (treewidth != nullptr);
                *treewidth = static_cast<unsigned>(sqlite3_column_int(*stmt, 0));
                if (*treewidth > 0) {
                    return true;
                }
                else {
                    /* min(treewidth) yields NULL if there is result, which converts to
                    * zero when extracted as int. Since a treewidth of zero is not
                    * possible, we can safely rely on 0 == NULL here. */
                    return false;
                }
            }
            else {
                return false;
            }
        }
        catch (const DbException& ex) {
            cerr << "WARN: " << ex.what() << "; retrying in 1 second" << endl;
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }
    }
    return false;
}

}  // ns db
}  // ns d2

