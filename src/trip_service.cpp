#include "trip_service.h"
#include <iostream>

TripService::TripService(Database& database) : db(database) {}

bool TripService::addTrip(int busId,
                          int routeId,
                          const std::string& departureDate,
                          const std::string& arrivalDate,
                          int passengerCount,
                          double ticketPrice) {
    const char* sql =
        "INSERT INTO trips "
        "(bus_id, route_id, departure_date, arrival_date, passenger_count, ticket_price) "
        "VALUES (?, ?, ?, ?, ?, ?);";

    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db.get(), sql, -1, &stmt, nullptr) != SQLITE_OK) {
        std::cerr << "Ошибка подготовки INSERT: " << db.getLastError() << "\n";
        return false;
    }

    sqlite3_bind_int(stmt, 1, busId);
    sqlite3_bind_int(stmt, 2, routeId);
    sqlite3_bind_text(stmt, 3, departureDate.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 4, arrivalDate.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_int(stmt, 5, passengerCount);
    sqlite3_bind_double(stmt, 6, ticketPrice);

    bool success = sqlite3_step(stmt) == SQLITE_DONE;
    if (!success) {
        std::cerr << "Ошибка добавления рейса: " << sqlite3_errmsg(db.get()) << "\n";
    }

    sqlite3_finalize(stmt);
    return success;
}

bool TripService::updateTrip(int tripId,
                             int busId,
                             int routeId,
                             const std::string& departureDate,
                             const std::string& arrivalDate,
                             int passengerCount,
                             double ticketPrice) {
    const char* sql =
        "UPDATE trips SET "
        "bus_id = ?, "
        "route_id = ?, "
        "departure_date = ?, "
        "arrival_date = ?, "
        "passenger_count = ?, "
        "ticket_price = ? "
        "WHERE trip_id = ?;";

    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db.get(), sql, -1, &stmt, nullptr) != SQLITE_OK) {
        std::cerr << "Ошибка подготовки UPDATE: " << db.getLastError() << "\n";
        return false;
    }

    sqlite3_bind_int(stmt, 1, busId);
    sqlite3_bind_int(stmt, 2, routeId);
    sqlite3_bind_text(stmt, 3, departureDate.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 4, arrivalDate.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_int(stmt, 5, passengerCount);
    sqlite3_bind_double(stmt, 6, ticketPrice);
    sqlite3_bind_int(stmt, 7, tripId);

    bool success = sqlite3_step(stmt) == SQLITE_DONE;
if (!success) {
    std::cerr << "Ошибка обновления рейса: " << sqlite3_errmsg(db.get()) << "\n";
} else if (sqlite3_changes(db.get()) == 0) {
    std::cerr << "Рейс с trip_id = " << tripId << " не найден.\n";
    success = false;
}

    sqlite3_finalize(stmt);
    return success;
}

bool TripService::deleteTrip(int tripId) {
    const char* sql = "DELETE FROM trips WHERE trip_id = ?;";

    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db.get(), sql, -1, &stmt, nullptr) != SQLITE_OK) {
        std::cerr << "Ошибка подготовки DELETE: " << db.getLastError() << "\n";
        return false;
    }

    sqlite3_bind_int(stmt, 1, tripId);

    bool success = sqlite3_step(stmt) == SQLITE_DONE;
if (!success) {
    std::cerr << "Ошибка удаления рейса: " << sqlite3_errmsg(db.get()) << "\n";
} else if (sqlite3_changes(db.get()) == 0) {
    std::cerr << "Рейс с trip_id = " << tripId << " не найден.\n";
    success = false;
}

    sqlite3_finalize(stmt);
    return success;
}

void TripService::printTripsByBusAndPeriod(const std::string& busNumber,
                                           const std::string& startDate,
                                           const std::string& endDate) {
    const char* sql =
        "SELECT t.trip_id, b.bus_number, r.route_name, t.departure_date, t.arrival_date, "
        "t.passenger_count, t.ticket_price "
        "FROM trips t "
        "JOIN buses b ON b.bus_id = t.bus_id "
        "JOIN routes r ON r.route_id = t.route_id "
        "WHERE b.bus_number = ? "
        "AND date(t.departure_date) BETWEEN date(?) AND date(?) "
        "ORDER BY t.departure_date;";

    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db.get(), sql, -1, &stmt, nullptr) != SQLITE_OK) {
        std::cerr << "Ошибка подготовки SELECT: " << db.getLastError() << "\n";
        return;
    }

    sqlite3_bind_text(stmt, 1, busNumber.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, startDate.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 3, endDate.c_str(), -1, SQLITE_TRANSIENT);

    std::cout << "\nРейсы автобуса " << busNumber << " за период " << startDate
              << " - " << endDate << ":\n";

    bool hasRows = false;
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        hasRows = true;
        std::cout
            << "ID рейса: " << sqlite3_column_int(stmt, 0)
            << ", автобус: " << reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1))
            << ", маршрут: " << reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2))
            << ", отправление: " << reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3))
            << ", прибытие: " << reinterpret_cast<const char*>(sqlite3_column_text(stmt, 4))
            << ", пассажиры: " << sqlite3_column_int(stmt, 5)
            << ", цена билета: " << sqlite3_column_double(stmt, 6)
            << "\n";
    }

    if (!hasRows) {
        std::cout << "Данные не найдены.\n";
    }

    sqlite3_finalize(stmt);
}