#include <gtest/gtest.h>
#include "database.h"
#include "trip_service.h"
#include <sqlite3.h>
#include <string>
#include <cstdio>
#include <functional>

class TripServiceTest : public ::testing::Test {
protected:
    Database db;

    void SetUp() override {
        std::remove(TEST_DB_PATH);
        ASSERT_TRUE(db.open(TEST_DB_PATH));
        ASSERT_TRUE(db.executeScriptFromFile(INIT_SQL_PATH));
    }

    void TearDown() override {
        db.close();
        std::remove(TEST_DB_PATH);
    }

    int getTripCount() {
        const char* sql = "SELECT COUNT(*) FROM trips;";
        sqlite3_stmt* stmt = nullptr;
        sqlite3_prepare_v2(db.get(), sql, -1, &stmt, nullptr);

        int count = 0;
        if (sqlite3_step(stmt) == SQLITE_ROW) {
            count = sqlite3_column_int(stmt, 0);
        }

        sqlite3_finalize(stmt);
        return count;
    }

    int getLatestTripIdByData(const std::string& departureDate,
                              const std::string& arrivalDate,
                              int passengerCount,
                              double ticketPrice) {
        const char* sql =
            "SELECT trip_id FROM trips "
            "WHERE departure_date = ? AND arrival_date = ? "
            "AND passenger_count = ? AND ticket_price = ? "
            "ORDER BY trip_id DESC LIMIT 1;";

        sqlite3_stmt* stmt = nullptr;
        sqlite3_prepare_v2(db.get(), sql, -1, &stmt, nullptr);
        sqlite3_bind_text(stmt, 1, departureDate.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 2, arrivalDate.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_int(stmt, 3, passengerCount);
        sqlite3_bind_double(stmt, 4, ticketPrice);

        int tripId = -1;
        if (sqlite3_step(stmt) == SQLITE_ROW) {
            tripId = sqlite3_column_int(stmt, 0);
        }

        sqlite3_finalize(stmt);
        return tripId;
    }

    std::string captureOutput(const std::function<void()>& func) {
        testing::internal::CaptureStdout();
        func();
        return testing::internal::GetCapturedStdout();
    }
};

TEST_F(TripServiceTest, AddTripSuccess) {
    TripService service(db);

    int before = getTripCount();

    bool result = service.addTrip(1, 1, "2024-11-01", "2024-11-01", 10, 1500.0);
    EXPECT_TRUE(result);

    int after = getTripCount();
    EXPECT_EQ(after, before + 1);
}

TEST_F(TripServiceTest, AddTripFailsWithInvalidBus) {
    TripService service(db);

    bool result = service.addTrip(999, 1, "2024-11-02", "2024-11-02", 10, 1500.0);
    EXPECT_FALSE(result);
}

TEST_F(TripServiceTest, AddTripFailsWithInvalidRoute) {
    TripService service(db);

    bool result = service.addTrip(1, 999, "2024-11-02", "2024-11-02", 10, 1500.0);
    EXPECT_FALSE(result);
}

TEST_F(TripServiceTest, AddTripFailsWhenArrivalBeforeDeparture) {
    TripService service(db);

    bool result = service.addTrip(1, 1, "2024-11-05", "2024-11-01", 10, 1500.0);
    EXPECT_FALSE(result);
}

TEST_F(TripServiceTest, AddTripFailsWithZeroPassengers) {
    TripService service(db);

    bool result = service.addTrip(1, 1, "2024-11-05", "2024-11-05", 0, 1500.0);
    EXPECT_FALSE(result);
}

TEST_F(TripServiceTest, AddTripFailsWithNegativeTicketPrice) {
    TripService service(db);

    bool result = service.addTrip(1, 1, "2024-11-05", "2024-11-05", 10, -10.0);
    EXPECT_FALSE(result);
}

TEST_F(TripServiceTest, DeleteTripSuccess) {
    TripService service(db);

    ASSERT_TRUE(service.addTrip(1, 1, "2024-11-03", "2024-11-03", 11, 1600.0));
    int tripId = getLatestTripIdByData("2024-11-03", "2024-11-03", 11, 1600.0);

    ASSERT_NE(tripId, -1);
    EXPECT_TRUE(service.deleteTrip(tripId));
}

TEST_F(TripServiceTest, DeleteTripRemovesRecordFromDatabase) {
    TripService service(db);

    ASSERT_TRUE(service.addTrip(1, 1, "2024-11-04", "2024-11-04", 13, 1650.0));
    int tripId = getLatestTripIdByData("2024-11-04", "2024-11-04", 13, 1650.0);

    ASSERT_NE(tripId, -1);
    ASSERT_TRUE(service.deleteTrip(tripId));

    EXPECT_EQ(getLatestTripIdByData("2024-11-04", "2024-11-04", 13, 1650.0), -1);
}

TEST_F(TripServiceTest, DeleteTripFailsForInvalidId) {
    TripService service(db);

    EXPECT_FALSE(service.deleteTrip(9999));
}

TEST_F(TripServiceTest, UpdateTripSuccess) {
    TripService service(db);

    ASSERT_TRUE(service.addTrip(1, 1, "2024-11-10", "2024-11-10", 12, 1700.0));
    int tripId = getLatestTripIdByData("2024-11-10", "2024-11-10", 12, 1700.0);

    ASSERT_NE(tripId, -1);

    bool result = service.updateTrip(tripId, 1, 2, "2024-11-11", "2024-11-11", 20, 1800.0);
    EXPECT_TRUE(result);
}

TEST_F(TripServiceTest, UpdateTripChangesStoredData) {
    TripService service(db);

    ASSERT_TRUE(service.addTrip(1, 1, "2024-11-12", "2024-11-12", 14, 1750.0));
    int tripId = getLatestTripIdByData("2024-11-12", "2024-11-12", 14, 1750.0);

    ASSERT_NE(tripId, -1);
    ASSERT_TRUE(service.updateTrip(tripId, 2, 2, "2024-11-13", "2024-11-13", 21, 1850.0));

    const char* sql =
        "SELECT bus_id, route_id, departure_date, arrival_date, passenger_count, ticket_price "
        "FROM trips WHERE trip_id = ?;";
    sqlite3_stmt* stmt = nullptr;
    sqlite3_prepare_v2(db.get(), sql, -1, &stmt, nullptr);
    sqlite3_bind_int(stmt, 1, tripId);

    ASSERT_EQ(sqlite3_step(stmt), SQLITE_ROW);

    EXPECT_EQ(sqlite3_column_int(stmt, 0), 2);
    EXPECT_EQ(sqlite3_column_int(stmt, 1), 2);
    EXPECT_EQ(std::string(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2))), "2024-11-13");
    EXPECT_EQ(std::string(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3))), "2024-11-13");
    EXPECT_EQ(sqlite3_column_int(stmt, 4), 21);
    EXPECT_EQ(sqlite3_column_double(stmt, 5), 1850.0);

    sqlite3_finalize(stmt);
}

TEST_F(TripServiceTest, UpdateTripFailsForInvalidId) {
    TripService service(db);

    bool result = service.updateTrip(9999, 1, 1, "2024-11-11", "2024-11-11", 20, 1800.0);
    EXPECT_FALSE(result);
}

TEST_F(TripServiceTest, UpdateTripFailsForInvalidBus) {
    TripService service(db);

    ASSERT_TRUE(service.addTrip(1, 1, "2024-11-14", "2024-11-14", 15, 1900.0));
    int tripId = getLatestTripIdByData("2024-11-14", "2024-11-14", 15, 1900.0);

    ASSERT_NE(tripId, -1);

    bool result = service.updateTrip(tripId, 999, 1, "2024-11-15", "2024-11-15", 16, 1950.0);
    EXPECT_FALSE(result);
}

TEST_F(TripServiceTest, UpdateTripFailsForInvalidRoute) {
    TripService service(db);

    ASSERT_TRUE(service.addTrip(1, 1, "2024-11-16", "2024-11-16", 17, 2000.0));
    int tripId = getLatestTripIdByData("2024-11-16", "2024-11-16", 17, 2000.0);

    ASSERT_NE(tripId, -1);

    bool result = service.updateTrip(tripId, 1, 999, "2024-11-17", "2024-11-17", 18, 2050.0);
    EXPECT_FALSE(result);
}

TEST_F(TripServiceTest, UpdateTripFailsWhenArrivalBeforeDeparture) {
    TripService service(db);

    ASSERT_TRUE(service.addTrip(1, 1, "2024-11-18", "2024-11-18", 19, 2100.0));
    int tripId = getLatestTripIdByData("2024-11-18", "2024-11-18", 19, 2100.0);

    ASSERT_NE(tripId, -1);

    bool result = service.updateTrip(tripId, 1, 1, "2024-11-20", "2024-11-19", 20, 2150.0);
    EXPECT_FALSE(result);
}

TEST_F(TripServiceTest, PrintTripsByBusAndPeriodShowsTrips) {
    TripService service(db);

    std::string output = captureOutput([&]() {
        service.printTripsByBusAndPeriod("A101", "2024-01-01", "2024-12-31");
    });

    EXPECT_NE(output.find("Рейсы автобуса A101"), std::string::npos);
    EXPECT_NE(output.find("ID рейса"), std::string::npos);
}

TEST_F(TripServiceTest, PrintTripsByBusAndPeriodShowsNotFoundWhenNoTrips) {
    TripService service(db);

    std::string output = captureOutput([&]() {
        service.printTripsByBusAndPeriod("A101", "2025-01-01", "2025-12-31");
    });

    EXPECT_NE(output.find("Данные не найдены"), std::string::npos);
}
