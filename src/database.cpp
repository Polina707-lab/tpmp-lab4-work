#include "database.h"
#include <fstream>
#include <iostream>
#include <sstream>

Database::Database() : db(nullptr) {}

Database::~Database() {
    close();
}

bool Database::open(const std::string& filename) {
    if (sqlite3_open(filename.c_str(), &db) != SQLITE_OK) {
        std::cerr << "Ошибка открытия БД: " << sqlite3_errmsg(db) << "\n";
        return false;
    }

    if (sqlite3_exec(db, "PRAGMA foreign_keys = ON;", nullptr, nullptr, nullptr) != SQLITE_OK) {
        std::cerr << "Не удалось включить foreign_keys.\n";
        return false;
    }

    return true;
}

void Database::close() {
    if (db != nullptr) {
        sqlite3_close(db);
        db = nullptr;
    }
}

bool Database::execute(const std::string& sql) {
    char* errMsg = nullptr;
    int rc = sqlite3_exec(db, sql.c_str(), nullptr, nullptr, &errMsg);

    if (rc != SQLITE_OK) {
        std::cerr << "SQL error: " << (errMsg ? errMsg : "unknown error") << "\n";
        sqlite3_free(errMsg);
        return false;
    }
    return true;
}

bool Database::executeScriptFromFile(const std::string& filepath) {
    std::ifstream file(filepath);
    if (!file.is_open()) {
        std::cerr << "Не удалось открыть SQL-файл: " << filepath << "\n";
        return false;
    }

    std::stringstream buffer;
    buffer << file.rdbuf();
    return execute(buffer.str());
}

bool Database::isInitialized() const {
    const char* sql =
        "SELECT name FROM sqlite_master "
        "WHERE type='table' AND name='users';";

    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        return false;
    }

    bool exists = (sqlite3_step(stmt) == SQLITE_ROW);
    sqlite3_finalize(stmt);
    return exists;
}

sqlite3* Database::get() const {
    return db;
}

std::string Database::getLastError() const {
    if (!db) {
        return "Database is not opened";
    }
    return sqlite3_errmsg(db);
}