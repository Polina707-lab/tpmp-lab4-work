#include <gtest/gtest.h>
#include "database.h"
#include "bus_service.h"
#include <sqlite3.h>
#include <string>

class BusServiceTest : public ::testing::Test {
protected:
    Database db;

    void SetUp() override {
        ASSERT_TRUE(db.open("data/tourist_bureau.db"));
    }

    void TearDown() override {
        db.close();
    }

    int getBusCount() {
        const char* sql = "SELECT COUNT(*) FROM buses;";
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

TEST_F(BusServiceTest, AddBusSuccess) {
    BusService service(db);

    int before = getBusCount();

    bool result = service.addBus("T999", "Test Bus", 12345.0);
    EXPECT_TRUE(result);

    int after = getBusCount();
    EXPECT_EQ(after, before + 1);

    db.execute("DELETE FROM buses WHERE bus_number = 'T999';");
}

TEST_F(BusServiceTest, UpdateBusSuccess) {
    BusService service(db);

    ASSERT_TRUE(service.addBus("T998", "Old Bus", 1000.0));

    const char* sql = "SELECT bus_id FROM buses WHERE bus_number = 'T998';";
    sqlite3_stmt* stmt = nullptr;
    sqlite3_prepare_v2(db.get(), sql, -1, &stmt, nullptr);

    int busId = -1;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        busId = sqlite3_column_int(stmt, 0);
    }

    sqlite3_finalize(stmt);
    ASSERT_NE(busId, -1);

    bool result = service.updateBus(busId, "T998", "Updated Bus", 2000.0);
    EXPECT_TRUE(result);

    db.execute("DELETE FROM buses WHERE bus_id = " + std::to_string(busId) + ";");
}

TEST_F(BusServiceTest, DeleteBusSuccess) {
    BusService service(db);

    ASSERT_TRUE(service.addBus("T997", "Delete Bus", 5000.0));

    const char* sql = "SELECT bus_id FROM buses WHERE bus_number = 'T997';";
    sqlite3_stmt* stmt = nullptr;
    sqlite3_prepare_v2(db.get(), sql, -1, &stmt, nullptr);

    int busId = -1;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        busId = sqlite3_column_int(stmt, 0);
    }

    sqlite3_finalize(stmt);
    ASSERT_NE(busId, -1);

    EXPECT_TRUE(service.deleteBus(busId));
}