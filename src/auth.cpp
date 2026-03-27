#include "auth.h"
#include <iostream>

AuthService::AuthService(Database& database) : db(database) {}

UserSession AuthService::login(const std::string& loginValue, const std::string& passwordValue) {
    UserSession session;

    const char* sql =
        "SELECT user_id, login, role, COALESCE(crew_member_id, -1) "
        "FROM users "
        "WHERE login = ? AND password = ?;";

    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db.get(), sql, -1, &stmt, nullptr) != SQLITE_OK) {
        std::cerr << "Ошибка подготовки SQL в login(): " << db.getLastError() << "\n";
        return session;
    }

    sqlite3_bind_text(stmt, 1, loginValue.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, passwordValue.c_str(), -1, SQLITE_TRANSIENT);

    if (sqlite3_step(stmt) == SQLITE_ROW) {
        session.authenticated = true;
        session.userId = sqlite3_column_int(stmt, 0);
        session.login = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
        session.role = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
        session.crewMemberId = sqlite3_column_int(stmt, 3);
    }

    sqlite3_finalize(stmt);
    return session;
}