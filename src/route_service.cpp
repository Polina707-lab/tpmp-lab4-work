#include "route_service.h"
#include <iostream>

RouteService::RouteService(Database& database) : db(database) {}

bool RouteService::addRoute(const std::string& routeName,
                            const std::string& startPoint,
                            const std::string& endPoint,
                            double distanceKm) {
    const char* sql =
        "INSERT INTO routes (route_name, start_point, end_point, distance_km) "
        "VALUES (?, ?, ?, ?);";

    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db.get(), sql, -1, &stmt, nullptr) != SQLITE_OK) {
        std::cerr << "Ошибка подготовки addRoute: " << db.getLastError() << "\n";
        return false;
    }

    sqlite3_bind_text(stmt, 1, routeName.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, startPoint.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 3, endPoint.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_double(stmt, 4, distanceKm);

    bool success = sqlite3_step(stmt) == SQLITE_DONE;
    if (!success) {
        std::cerr << "Ошибка добавления маршрута: " << sqlite3_errmsg(db.get()) << "\n";
    }

    sqlite3_finalize(stmt);
    return success;
}

bool RouteService::updateRoute(int routeId,
                               const std::string& routeName,
                               const std::string& startPoint,
                               const std::string& endPoint,
                               double distanceKm) {
    const char* sql =
        "UPDATE routes SET "
        "route_name = ?, "
        "start_point = ?, "
        "end_point = ?, "
        "distance_km = ? "
        "WHERE route_id = ?;";

    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db.get(), sql, -1, &stmt, nullptr) != SQLITE_OK) {
        std::cerr << "Ошибка подготовки updateRoute: " << db.getLastError() << "\n";
        return false;
    }

    sqlite3_bind_text(stmt, 1, routeName.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, startPoint.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 3, endPoint.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_double(stmt, 4, distanceKm);
    sqlite3_bind_int(stmt, 5, routeId);

    bool success = sqlite3_step(stmt) == SQLITE_DONE;
    if (!success) {
        std::cerr << "Ошибка обновления маршрута: " << sqlite3_errmsg(db.get()) << "\n";
    } else if (sqlite3_changes(db.get()) == 0) {
        std::cerr << "Маршрут с route_id = " << routeId << " не найден.\n";
        success = false;
    }

    sqlite3_finalize(stmt);
    return success;
}

bool RouteService::deleteRoute(int routeId) {
    const char* sql = "DELETE FROM routes WHERE route_id = ?;";

    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db.get(), sql, -1, &stmt, nullptr) != SQLITE_OK) {
        std::cerr << "Ошибка подготовки deleteRoute: " << db.getLastError() << "\n";
        return false;
    }

    sqlite3_bind_int(stmt, 1, routeId);

    bool success = sqlite3_step(stmt) == SQLITE_DONE;
    if (!success) {
        std::cerr << "Ошибка удаления маршрута: " << sqlite3_errmsg(db.get()) << "\n";
    } else if (sqlite3_changes(db.get()) == 0) {
        std::cerr << "Маршрут с route_id = " << routeId << " не найден.\n";
        success = false;
    }

    sqlite3_finalize(stmt);
    return success;
}

void RouteService::printAllRoutes() {
    const char* sql =
        "SELECT route_id, route_name, start_point, end_point, distance_km "
        "FROM routes "
        "ORDER BY route_id;";

    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db.get(), sql, -1, &stmt, nullptr) != SQLITE_OK) {
        std::cerr << "Ошибка подготовки printAllRoutes: " << db.getLastError() << "\n";
        return;
    }

    std::cout << "\nСписок маршрутов:\n";

    bool hasRows = false;
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        hasRows = true;
        std::cout
            << "ID: " << sqlite3_column_int(stmt, 0)
            << ", маршрут: " << reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1))
            << ", откуда: " << reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2))
            << ", куда: " << reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3))
            << ", длина: " << sqlite3_column_double(stmt, 4)
            << " км\n";
    }

    if (!hasRows) {
        std::cout << "Маршруты не найдены.\n";
    }

    sqlite3_finalize(stmt);
}