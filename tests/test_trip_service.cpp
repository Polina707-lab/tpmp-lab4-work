#include <gtest/gtest.h>
#include "database.h"
#include "trip_service.h"
#include <sqlite3.h>
#include <string>

class TripServiceTest : public ::testing::Test {
protected:
    Database db;

    void SetUp() override {
        ASSERT_TRUE(db.open("data/tourist_bureau.db"));
    }

    void TearDown() override {
        db.close();
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

    db.execute("DELETE FROM trips WHERE departure_date = '2024-11-01' AND arrival_date = '2024-11-01' AND passenger_count = 10 AND ticket_price = 1500.0;");
}

TEST_F(TripServiceTest, AddTripFailsWithInvalidBus) {
    TripService service(db);

    bool result = service.addTrip(999, 1, "2024-11-02", "2024-11-02", 10, 1500.0);
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