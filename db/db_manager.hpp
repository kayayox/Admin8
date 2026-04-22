#ifndef DB_MANAGER_HPP
#define DB_MANAGER_HPP

#include <sqlite3.h>
#include <string>
#include <iostream>

class Database {
public:
    static Database& instance();
    bool init(const std::string& path);
    void close();
    sqlite3* getHandle() const { return db_; }

    // No copiable
    Database(const Database&) = delete;
    Database& operator=(const Database&) = delete;

private:
    Database() = default;
    ~Database() { close(); }
    sqlite3* db_ = nullptr;
};

inline bool prepareStatement(sqlite3* db, const char* sql, sqlite3_stmt** stmt) {
    int rc = sqlite3_prepare_v2(db, sql, -1, stmt, nullptr);
    if (rc != SQLITE_OK) {
        std::cerr << "Error preparando SQL: " << sql << "\n"
                  << "Mensaje: " << sqlite3_errmsg(db) << std::endl;
        return false;
    }
    return true;
}

#endif // DB_MANAGER_HPP
