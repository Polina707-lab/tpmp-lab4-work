#include "bus_service.h"
#include <iostream>

BusService::BusService(Database& database) : db(database) {}

bool BusService::addBus(const std::string& busNumber,
                        const std::string& busName,
                        double totalMileageKm) {
    const char* sql =
        "INSERT INTO buses (bus_number, bus_name, total_mileage_km) "
        "VALUES (?, ?, ?);";

    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db.get(), sql, -1, &stmt, nullptr) != SQLITE_OK) {
        std::cerr << "Ошибка подготовки addBus: " << db.getLastError() << "\n";
        return false;
    }

    sqlite3_bind_text(stmt, 1, busNumber.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, busName.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_double(stmt, 3, totalMileageKm);

    bool success = sqlite3_step(stmt) == SQLITE_DONE;
    if (!success) {
        std::cerr << "Ошибка добавления автобуса: " << sqlite3_errmsg(db.get()) << "\n";
    }

    sqlite3_finalize(stmt);
    return success;
}

bool BusService::updateBus(int busId,
                           const std::string& busNumber,
                           const std::string& busName,
                           double totalMileageKm) {
    const char* sql =
        "UPDATE buses SET "
        "bus_number = ?, "
        "bus_name = ?, "
        "total_mileage_km = ? "
        "WHERE bus_id = ?;";

    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db.get(), sql, -1, &stmt, nullptr) != SQLITE_OK) {
        std::cerr << "Ошибка подготовки updateBus: " << db.getLastError() << "\n";
        return false;
    }

    sqlite3_bind_text(stmt, 1, busNumber.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, busName.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_double(stmt, 3, totalMileageKm);
    sqlite3_bind_int(stmt, 4, busId);

    bool success = sqlite3_step(stmt) == SQLITE_DONE;
    if (!success) {
        std::cerr << "Ошибка обновления автобуса: " << sqlite3_errmsg(db.get()) << "\n";
    } else if (sqlite3_changes(db.get()) == 0) {
        std::cerr << "Автобус с bus_id = " << busId << " не найден.\n";
        success = false;
    }

    sqlite3_finalize(stmt);
    return success;
}

bool BusService::deleteBus(int busId) {
    const char* sql = "DELETE FROM buses WHERE bus_id = ?;";

    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db.get(), sql, -1, &stmt, nullptr) != SQLITE_OK) {
        std::cerr << "Ошибка подготовки deleteBus: " << db.getLastError() << "\n";
        return false;
    }

    sqlite3_bind_int(stmt, 1, busId);

    bool success = sqlite3_step(stmt) == SQLITE_DONE;
    if (!success) {
        std::cerr << "Ошибка удаления автобуса: " << sqlite3_errmsg(db.get()) << "\n";
    } else if (sqlite3_changes(db.get()) == 0) {
        std::cerr << "Автобус с bus_id = " << busId << " не найден.\n";
        success = false;
    }

    sqlite3_finalize(stmt);
    return success;
}

void BusService::printAllBuses() {
    const char* sql =
        "SELECT bus_id, bus_number, bus_name, total_mileage_km, LENGTH(image_data) "
        "FROM buses "
        "ORDER BY bus_id;";

    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db.get(), sql, -1, &stmt, nullptr) != SQLITE_OK) {
        std::cerr << "Ошибка подготовки printAllBuses: " << db.getLastError() << "\n";
        return;
    }

    std::cout << "\nСписок автобусов:\n";

    bool hasRows = false;
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        hasRows = true;
        int blobSize = sqlite3_column_type(stmt, 4) == SQLITE_NULL ? 0 : sqlite3_column_int(stmt, 4);

        std::cout
            << "ID: " << sqlite3_column_int(stmt, 0)
            << ", номер: " << reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1))
            << ", название: " << reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2))
            << ", пробег: " << sqlite3_column_double(stmt, 3)
            << ", изображение: " << (blobSize > 0 ? "есть" : "нет")
            << "\n";
    }

    if (!hasRows) {
        std::cout << "Автобусы не найдены.\n";
    }

    sqlite3_finalize(stmt);
}