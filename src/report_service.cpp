#include "report_service.h"
#include <iostream>
#include <fstream>
#include <vector>

ReportService::ReportService(Database& database) : db(database) {}

int ReportService::getCrewIdByMemberId(int crewMemberId) {
    const char* sql = "SELECT crew_id FROM crew_members WHERE member_id = ?;";
    sqlite3_stmt* stmt = nullptr;

    if (sqlite3_prepare_v2(db.get(), sql, -1, &stmt, nullptr) != SQLITE_OK) {
        return -1;
    }

    sqlite3_bind_int(stmt, 1, crewMemberId);

    int crewId = -1;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        crewId = sqlite3_column_int(stmt, 0);
    }

    sqlite3_finalize(stmt);
    return crewId;
}

void ReportService::printBusSummary(const std::string& busNumber,
                                    const std::string& startDate,
                                    const std::string& endDate) {
    const char* sql =
        "SELECT b.bus_number, "
        "COUNT(t.trip_id) AS trip_count, "
        "COALESCE(SUM(t.passenger_count), 0), "
        "COALESCE(SUM(t.passenger_count * t.ticket_price), 0) "
        "FROM buses b "
        "LEFT JOIN trips t ON t.bus_id = b.bus_id "
        "AND date(t.departure_date) BETWEEN date(?) AND date(?) "
        "WHERE b.bus_number = ? "
        "GROUP BY b.bus_id, b.bus_number;";

    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db.get(), sql, -1, &stmt, nullptr) != SQLITE_OK) {
        std::cerr << "Ошибка подготовки summary: " << db.getLastError() << "\n";
        return;
    }

    sqlite3_bind_text(stmt, 1, startDate.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, endDate.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 3, busNumber.c_str(), -1, SQLITE_TRANSIENT);

    if (sqlite3_step(stmt) == SQLITE_ROW) {
        std::cout << "\nСтатистика по автобусу " 
                  << reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0)) << ":\n"
                  << "Количество поездок: " << sqlite3_column_int(stmt, 1) << "\n"
                  << "Количество пассажиров: " << sqlite3_column_int(stmt, 2) << "\n"
                  << "Получено денег: " << sqlite3_column_double(stmt, 3) << "\n";
    } else {
        std::cout << "Данные не найдены.\n";
    }

    sqlite3_finalize(stmt);
}

void ReportService::printCrewIncomeByPeriod(const std::string& startDate,
                                            const std::string& endDate) {
    const char* sql =
        "SELECT c.crew_id, b.bus_number, "
        "ROUND(COALESCE(SUM(CASE "
        "    WHEN date(t.departure_date) BETWEEN date(?) AND date(?) "
        "    THEN t.passenger_count * t.ticket_price * 0.20 "
        "    ELSE 0 "
        "END), 0), 2) AS income "
        "FROM crews c "
        "JOIN buses b ON b.bus_id = c.bus_id "
        "LEFT JOIN trips t ON t.bus_id = c.bus_id "
        "GROUP BY c.crew_id, b.bus_number "
        "ORDER BY c.crew_id;";

    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db.get(), sql, -1, &stmt, nullptr) != SQLITE_OK) {
        std::cerr << "Ошибка подготовки отчета по экипажам: "
                  << db.getLastError() << "\n";
        return;
    }

    sqlite3_bind_text(stmt, 1, startDate.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, endDate.c_str(), -1, SQLITE_TRANSIENT);

    std::cout << "\nНачисления по каждому экипажу за период:\n";

    bool hasRows = false;
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        hasRows = true;
        std::cout
            << "Экипаж ID: " << sqlite3_column_int(stmt, 0)
            << ", автобус: " << reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1))
            << ", начислено: " << sqlite3_column_double(stmt, 2)
            << "\n";
    }

    if (!hasRows) {
        std::cout << "Данные не найдены.\n";
    }

    sqlite3_finalize(stmt);
}

bool ReportService::calculateCrewPayments(double percentRate,
                                          const std::string& startDate,
                                          const std::string& endDate) {
    const char* deleteSql =
        "DELETE FROM crew_payments "
        "WHERE period_start = ? AND period_end = ? AND percent_rate = ?;";

    sqlite3_stmt* deleteStmt = nullptr;
    if (sqlite3_prepare_v2(db.get(), deleteSql, -1, &deleteStmt, nullptr) != SQLITE_OK) {
        std::cerr << "Ошибка подготовки DELETE crew_payments: " << db.getLastError() << "\n";
        return false;
    }

    sqlite3_bind_text(deleteStmt, 1, startDate.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(deleteStmt, 2, endDate.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_double(deleteStmt, 3, percentRate);

    if (sqlite3_step(deleteStmt) != SQLITE_DONE) {
        std::cerr << "Ошибка удаления старых начислений: " << sqlite3_errmsg(db.get()) << "\n";
        sqlite3_finalize(deleteStmt);
        return false;
    }
    sqlite3_finalize(deleteStmt);

    const char* insertSql =
        "INSERT INTO crew_payments (crew_id, period_start, period_end, percent_rate, total_amount) "
        "SELECT c.crew_id, ?, ?, ?, "
        "ROUND(COALESCE(SUM(t.passenger_count * t.ticket_price * (? / 100.0)), 0), 2) "
        "FROM crews c "
        "LEFT JOIN trips t ON t.bus_id = c.bus_id "
        "AND date(t.departure_date) BETWEEN date(?) AND date(?) "
        "GROUP BY c.crew_id;";

    sqlite3_stmt* insertStmt = nullptr;
    if (sqlite3_prepare_v2(db.get(), insertSql, -1, &insertStmt, nullptr) != SQLITE_OK) {
        std::cerr << "Ошибка подготовки calculateCrewPayments: " << db.getLastError() << "\n";
        return false;
    }

    sqlite3_bind_text(insertStmt, 1, startDate.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(insertStmt, 2, endDate.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_double(insertStmt, 3, percentRate);
    sqlite3_bind_double(insertStmt, 4, percentRate);
    sqlite3_bind_text(insertStmt, 5, startDate.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(insertStmt, 6, endDate.c_str(), -1, SQLITE_TRANSIENT);

    bool success = sqlite3_step(insertStmt) == SQLITE_DONE;
    if (!success) {
        std::cerr << "Ошибка расчета начислений: " << sqlite3_errmsg(db.get()) << "\n";
    }

    sqlite3_finalize(insertStmt);
    return success;
}

void ReportService::printCrewPaymentOnDate(int crewId, const std::string& dateValue) {
    const char* sql =
        "SELECT crew_id, period_start, period_end, percent_rate, total_amount, calc_date "
        "FROM crew_payments "
        "WHERE crew_id = ? "
        "AND date(?) BETWEEN date(period_start) AND date(period_end) "
        "ORDER BY payment_id DESC;";

    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db.get(), sql, -1, &stmt, nullptr) != SQLITE_OK) {
        std::cerr << "Ошибка подготовки printCrewPaymentOnDate: " << db.getLastError() << "\n";
        return;
    }

    sqlite3_bind_int(stmt, 1, crewId);
    sqlite3_bind_text(stmt, 2, dateValue.c_str(), -1, SQLITE_TRANSIENT);

    std::cout << "\nНачисления экипажу " << crewId << " на дату " << dateValue << ":\n";

    bool hasRows = false;
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        hasRows = true;
        std::cout
            << "Период: " << reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1))
            << " - " << reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2))
            << ", процент: " << sqlite3_column_double(stmt, 3)
            << ", сумма: " << sqlite3_column_double(stmt, 4)
            << ", дата расчета: " << reinterpret_cast<const char*>(sqlite3_column_text(stmt, 5))
            << "\n";
    }

    if (!hasRows) {
        std::cout << "Начисления не найдены.\n";
    }

    sqlite3_finalize(stmt);
}

void ReportService::printMostExpensiveRouteInfo() {
    const char* sql =
        "SELECT r.route_name, b.bus_number, t.ticket_price, c.crew_id, cm.surname "
        "FROM trips t "
        "JOIN routes r ON r.route_id = t.route_id "
        "JOIN buses b ON b.bus_id = t.bus_id "
        "JOIN crews c ON c.bus_id = b.bus_id "
        "JOIN crew_members cm ON cm.crew_id = c.crew_id "
        "WHERE t.ticket_price = (SELECT MAX(ticket_price) FROM trips) "
        "ORDER BY r.route_name, c.crew_id, cm.surname;";

    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db.get(), sql, -1, &stmt, nullptr) != SQLITE_OK) {
        std::cerr << "Ошибка подготовки printMostExpensiveRouteInfo: " << db.getLastError() << "\n";
        return;
    }

    std::cout << "\nСведения по наиболее дорогому маршруту:\n";
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        std::cout
            << "Маршрут: " << reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0))
            << ", автобус: " << reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1))
            << ", цена билета: " << sqlite3_column_double(stmt, 2)
            << ", экипаж ID: " << sqlite3_column_int(stmt, 3)
            << ", член экипажа: " << reinterpret_cast<const char*>(sqlite3_column_text(stmt, 4))
            << "\n";
    }

    sqlite3_finalize(stmt);
}

void ReportService::printTopMileageBusInfo() {
    const char* sql =
        "SELECT b.bus_number, b.bus_name, b.total_mileage_km, "
        "COALESCE(SUM(t.passenger_count), 0) AS passengers, "
        "LENGTH(b.image_data) "
        "FROM buses b "
        "LEFT JOIN trips t ON t.bus_id = b.bus_id "
        "WHERE b.total_mileage_km = (SELECT MAX(total_mileage_km) FROM buses) "
        "GROUP BY b.bus_id, b.bus_number, b.bus_name, b.total_mileage_km, b.image_data;";

    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db.get(), sql, -1, &stmt, nullptr) != SQLITE_OK) {
        std::cerr << "Ошибка подготовки printTopMileageBusInfo: " << db.getLastError() << "\n";
        return;
    }

    std::cout << "\nАвтобус с наибольшим суммарным пробегом:\n";
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        int blobSize = sqlite3_column_type(stmt, 4) == SQLITE_NULL ? 0 : sqlite3_column_int(stmt, 4);

        std::cout
            << "Номер автобуса: " << reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0))
            << ", название: " << reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1))
            << ", пробег: " << sqlite3_column_double(stmt, 2)
            << ", перевезено пассажиров: " << sqlite3_column_int(stmt, 3)
            << ", изображение: " << (blobSize > 0 ? "есть" : "нет")
            << "\n";
    }

    sqlite3_finalize(stmt);
}

void ReportService::printCrewMemberOwnInfo(int crewMemberId) {
    const char* sql =
        "SELECT crew_id, bus_number, bus_name, member_id, surname, personnel_number, "
        "experience_years, category, address, birth_year "
        "FROM crew_full_info "
        "WHERE member_id = ?;";

    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db.get(), sql, -1, &stmt, nullptr) != SQLITE_OK) {
        std::cerr << "Ошибка подготовки printCrewMemberOwnInfo: " << db.getLastError() << "\n";
        return;
    }

    sqlite3_bind_int(stmt, 1, crewMemberId);

    if (sqlite3_step(stmt) == SQLITE_ROW) {
        std::cout << "\nВаши данные:\n"
                  << "Экипаж ID: " << sqlite3_column_int(stmt, 0) << "\n"
                  << "Автобус: " << reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1))
                  << " (" << reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2)) << ")\n"
                  << "ID сотрудника: " << sqlite3_column_int(stmt, 3) << "\n"
                  << "Фамилия: " << reinterpret_cast<const char*>(sqlite3_column_text(stmt, 4)) << "\n"
                  << "Табельный номер: " << reinterpret_cast<const char*>(sqlite3_column_text(stmt, 5)) << "\n"
                  << "Стаж: " << sqlite3_column_int(stmt, 6) << "\n"
                  << "Категория: " << reinterpret_cast<const char*>(sqlite3_column_text(stmt, 7)) << "\n"
                  << "Адрес: " << reinterpret_cast<const char*>(sqlite3_column_text(stmt, 8)) << "\n"
                  << "Год рождения: " << sqlite3_column_int(stmt, 9) << "\n";
    } else {
        std::cout << "Информация о пользователе не найдена.\n";
    }

    sqlite3_finalize(stmt);
}


bool ReportService::saveBusImageFromFile(const std::string& busNumber, const std::string& filePath) {
    std::ifstream file(filePath, std::ios::binary);
    if (!file.is_open()) {
        std::cerr << "Не удалось открыть файл изображения: " << filePath << "\n";
        return false;
    }

    std::vector<char> imageData((std::istreambuf_iterator<char>(file)),
                                 std::istreambuf_iterator<char>());

    const char* sql =
        "UPDATE buses "
        "SET image_data = ? "
        "WHERE bus_number = ?;";

    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db.get(), sql, -1, &stmt, nullptr) != SQLITE_OK) {
        std::cerr << "Ошибка подготовки saveBusImageFromFile: " << db.getLastError() << "\n";
        return false;
    }

    sqlite3_bind_blob(stmt, 1, imageData.data(), static_cast<int>(imageData.size()), SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, busNumber.c_str(), -1, SQLITE_TRANSIENT);

    bool success = sqlite3_step(stmt) == SQLITE_DONE;
if (!success) {
    std::cerr << "Ошибка сохранения изображения: " << sqlite3_errmsg(db.get()) << "\n";
} else if (sqlite3_changes(db.get()) == 0) {
    std::cerr << "Автобус с номером " << busNumber << " не найден.\n";
    success = false;
}

sqlite3_finalize(stmt);
return success;

}

bool ReportService::exportBusImageToFile(const std::string& busNumber, const std::string& outputPath) {
    const char* sql =
        "SELECT image_data "
        "FROM buses "
        "WHERE bus_number = ?;";

    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db.get(), sql, -1, &stmt, nullptr) != SQLITE_OK) {
        std::cerr << "Ошибка подготовки exportBusImageToFile: " << db.getLastError() << "\n";
        return false;
    }

    sqlite3_bind_text(stmt, 1, busNumber.c_str(), -1, SQLITE_TRANSIENT);

    bool success = false;

    if (sqlite3_step(stmt) == SQLITE_ROW) {
        const void* blob = sqlite3_column_blob(stmt, 0);
        int size = sqlite3_column_bytes(stmt, 0);

        if (blob && size > 0) {
            std::ofstream out(outputPath, std::ios::binary);
            if (!out.is_open()) {
                std::cerr << "Не удалось создать файл: " << outputPath << "\n";
            } else {
                out.write(static_cast<const char*>(blob), size);
                success = true;
            }
        } else {
            std::cerr << "Изображение для автобуса не найдено.\n";
        }
    } else {
        std::cerr << "Автобус не найден.\n";
    }

    sqlite3_finalize(stmt);
    return success;
}

void ReportService::printBusImageInfo(const std::string& busNumber) {
    const char* sql =
        "SELECT bus_number, bus_name, image_data "
        "FROM buses "
        "WHERE bus_number = ?;";

    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db.get(), sql, -1, &stmt, nullptr) != SQLITE_OK) {
        std::cerr << "Ошибка подготовки printBusImageInfo: " << db.getLastError() << "\n";
        return;
    }

    sqlite3_bind_text(stmt, 1, busNumber.c_str(), -1, SQLITE_TRANSIENT);

    if (sqlite3_step(stmt) == SQLITE_ROW) {
        int blobSize = sqlite3_column_bytes(stmt, 2);

        std::cout << "\nИнформация об изображении автобуса:\n"
                  << "Номер автобуса: " << reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0)) << "\n"
                  << "Название: " << reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1)) << "\n"
                  << "Изображение: " << (blobSize > 0 ? "загружено" : "отсутствует") << "\n"
                  << "Размер BLOB: " << blobSize << " байт\n";
    } else {
        std::cout << "Автобус не найден.\n";
    }

    sqlite3_finalize(stmt);
}