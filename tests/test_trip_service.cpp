#include <gtest/gtest.h>
#include "database.h"
#include "trip_service.h"
#include <sqlite3.h>
#include <string>
#include <cstdio>

class TripServiceTest : public ::testing::Test {
protected:
    Database db;

    void SetUp() override {
        ASSERT_TRUE(db.open("data/test.db"));
        ASSERT_TRUE(db.executeScriptFromFile("sql/init.sql"));
    }

    void TearDown() override {
        db.close();
        std::remove("data/test.db");
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

TEST_F(TripServiceTest, DeleteTripSuccess) {
    TripService service(db);

    ASSERT_TRUE(service.addTrip(1, 1, "2024-11-03", "2024-11-03", 11, 1600.0));

    const char* sql =
        "SELECT trip_id FROM trips "
        "WHERE departure_date = '2024-11-03' AND arrival_date = '2024-11-03' "
        "AND passenger_count = 11 AND ticket_price = 1600.0 "
        "ORDER BY trip_id DESC LIMIT 1;";

    sqlite3_stmt* stmt = nullptr;
    sqlite3_prepare_v2(db.get(), sql, -1, &stmt, nullptr);

    int tripId = -1;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        tripId = sqlite3_column_int(stmt, 0);
    }
    sqlite3_finalize(stmt);

    ASSERT_NE(tripId, -1);
    EXPECT_TRUE(service.deleteTrip(tripId));
}

TEST_F(TripServiceTest, DeleteTripFailsForInvalidId) {
    TripService service(db);

    EXPECT_FALSE(service.deleteTrip(9999));
}

TEST_F(TripServiceTest, UpdateTripSuccess) {
    TripService service(db);

    ASSERT_TRUE(service.addTrip(1, 1, "2024-11-10", "2024-11-10", 12, 1700.0));

    const char* sql =
        "SELECT trip_id FROM trips "
        "WHERE departure_date = '2024-11-10' AND arrival_date = '2024-11-10' "
        "AND passenger_count = 12 AND ticket_price = 1700.0 "
        "ORDER BY trip_id DESC LIMIT 1;";

    sqlite3_stmt* stmt = nullptr;
    sqlite3_prepare_v2(db.get(), sql, -1, &stmt, nullptr);

    int tripId = -1;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        tripId = sqlite3_column_int(stmt, 0);
    }
    sqlite3_finalize(stmt);

    ASSERT_NE(tripId, -1);

    bool result = service.updateTrip(tripId, 1, 2, "2024-11-11", "2024-11-11", 20, 1800.0);
    EXPECT_TRUE(result);
}

TEST_F(TripServiceTest, UpdateTripFailsForInvalidId) {
    TripService service(db);

    bool result = service.updateTrip(9999, 1, 1, "2024-11-11", "2024-11-11", 20, 1800.0);
    EXPECT_FALSE(result);
}