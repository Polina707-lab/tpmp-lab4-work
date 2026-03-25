#pragma once
#include <sqlite3.h>
#include <string>

class Database {
private:
    sqlite3* db;

public:
    Database();
    ~Database();

    bool open(const std::string& filename);
    void close();

    bool execute(const std::string& sql);
    bool executeScriptFromFile(const std::string& filepath);
    bool isInitialized() const;

    sqlite3* get() const;
    std::string getLastError() const;
};