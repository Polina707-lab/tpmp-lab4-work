#include <gtest/gtest.h>
#include "database.h"
#include "bus_service.h"
#include <sqlite3.h>
#include <string>
#include <cstdio>

class BusServiceTest : public ::testing::Test {
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
}

TEST_F(BusServiceTest, AddBusFailsWithDuplicateNumber) {
    BusService service(db);

    bool result = service.addBus("A101", "Duplicate Bus", 1000.0);
    EXPECT_FALSE(result);
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
}

TEST_F(BusServiceTest, UpdateBusFailsForInvalidId) {
    BusService service(db);

    bool result = service.updateBus(9999, "X", "Invalid", 100.0);
    EXPECT_FALSE(result);
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

TEST_F(BusServiceTest, DeleteBusFailsForInvalidId) {
    BusService service(db);

    EXPECT_FALSE(service.deleteBus(9999));
}

TEST_F(BusServiceTest, AddBusStoresCorrectData) {
    BusService service(db);

    ASSERT_TRUE(service.addBus("T996", "Check Bus", 7777.0));

    const char* sql = "SELECT bus_name, total_mileage_km FROM buses WHERE bus_number = 'T996';";
    sqlite3_stmt* stmt = nullptr;
    sqlite3_prepare_v2(db.get(), sql, -1, &stmt, nullptr);

    ASSERT_EQ(sqlite3_step(stmt), SQLITE_ROW);

    std::string name = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
    double mileage = sqlite3_column_double(stmt, 1);

    sqlite3_finalize(stmt);

    EXPECT_EQ(name, "Check Bus");
    EXPECT_EQ(mileage, 7777.0);
}