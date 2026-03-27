#pragma once
#include "database.h"
#include "models.h"
#include <string>

class AuthService {
private:
    Database& db;

public:
    explicit AuthService(Database& database);

    UserSession login(const std::string& login, const std::string& password);
};