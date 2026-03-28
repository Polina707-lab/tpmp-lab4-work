#include <gtest/gtest.h>
#include "database.h"
#include "bus_service.h"
#include <sqlite3.h>
#include <string>
#include <cstdio>
#include <functional>

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

    int getBusIdByNumber(const std::string& busNumber) {
        const char* sql = "SELECT bus_id FROM buses WHERE bus_number = ?;";
        sqlite3_stmt* stmt = nullptr;
        sqlite3_prepare_v2(db.get(), sql, -1, &stmt, nullptr);
        sqlite3_bind_text(stmt, 1, busNumber.c_str(), -1, SQLITE_TRANSIENT);

        int busId = -1;
        if (sqlite3_step(stmt) == SQLITE_ROW) {
            busId = sqlite3_column_int(stmt, 0);
        }

        sqlite3_finalize(stmt);
        return busId;
    }

    std::string captureOutput(const std::function<void()>& func) {
        testing::internal::CaptureStdout();
        func();
        return testing::internal::GetCapturedStdout();
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

TEST_F(BusServiceTest, AddBusFailsWithNegativeMileage) {
    BusService service(db);

    bool result = service.addBus("T995", "Invalid Bus", -100.0);
    EXPECT_FALSE(result);
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

TEST_F(BusServiceTest, UpdateBusSuccess) {
    BusService service(db);

    ASSERT_TRUE(service.addBus("T998", "Old Bus", 1000.0));
    int busId = getBusIdByNumber("T998");
    ASSERT_NE(busId, -1);

    bool result = service.updateBus(busId, "T998", "Updated Bus", 2000.0);
    EXPECT_TRUE(result);
}

TEST_F(BusServiceTest, UpdateBusChangesStoredData) {
    BusService service(db);

    ASSERT_TRUE(service.addBus("T994", "Before Update", 1500.0));
    int busId = getBusIdByNumber("T994");
    ASSERT_NE(busId, -1);

    ASSERT_TRUE(service.updateBus(busId, "T994", "After Update", 2500.0));

    const char* sql = "SELECT bus_name, total_mileage_km FROM buses WHERE bus_id = ?;";
    sqlite3_stmt* stmt = nullptr;
    sqlite3_prepare_v2(db.get(), sql, -1, &stmt, nullptr);
    sqlite3_bind_int(stmt, 1, busId);

    ASSERT_EQ(sqlite3_step(stmt), SQLITE_ROW);

    std::string name = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
    double mileage = sqlite3_column_double(stmt, 1);

    sqlite3_finalize(stmt);

    EXPECT_EQ(name, "After Update");
    EXPECT_EQ(mileage, 2500.0);
}

TEST_F(BusServiceTest, UpdateBusFailsForInvalidId) {
    BusService service(db);

    bool result = service.updateBus(9999, "X", "Invalid", 100.0);
    EXPECT_FALSE(result);
}

TEST_F(BusServiceTest, UpdateBusFailsWithDuplicateNumber) {
    BusService service(db);

    ASSERT_TRUE(service.addBus("T993", "Another Bus", 1200.0));
    int busId = getBusIdByNumber("T993");
    ASSERT_NE(busId, -1);

    bool result = service.updateBus(busId, "A101", "Conflict Bus", 1300.0);
    EXPECT_FALSE(result);
}

TEST_F(BusServiceTest, DeleteBusSuccess) {
    BusService service(db);

    ASSERT_TRUE(service.addBus("T997", "Delete Bus", 5000.0));
    int busId = getBusIdByNumber("T997");
    ASSERT_NE(busId, -1);

    EXPECT_TRUE(service.deleteBus(busId));
}

TEST_F(BusServiceTest, DeleteBusRemovesRecordFromDatabase) {
    BusService service(db);

    ASSERT_TRUE(service.addBus("T992", "Removable Bus", 3000.0));
    int busId = getBusIdByNumber("T992");
    ASSERT_NE(busId, -1);

    ASSERT_TRUE(service.deleteBus(busId));

    EXPECT_EQ(getBusIdByNumber("T992"), -1);
}

TEST_F(BusServiceTest, DeleteBusFailsForInvalidId) {
    BusService service(db);

    EXPECT_FALSE(service.deleteBus(9999));
}

TEST_F(BusServiceTest, PrintAllBusesShowsExistingBuses) {
    BusService service(db);

    std::string output = captureOutput([&]() {
        service.printAllBuses();
    });

    EXPECT_NE(output.find("Список автобусов"), std::string::npos);
    EXPECT_NE(output.find("A101"), std::string::npos);
}

TEST_F(BusServiceTest, PrintAllBusesShowsMessageWhenNoBusesExist) {
    ASSERT_TRUE(db.execute("DELETE FROM trips;"));
    ASSERT_TRUE(db.execute("DELETE FROM users;"));
    ASSERT_TRUE(db.execute("DELETE FROM crew_members;"));
    ASSERT_TRUE(db.execute("DELETE FROM crews;"));
    ASSERT_TRUE(db.execute("DELETE FROM buses;"));

    BusService service(db);

    std::string output = captureOutput([&]() {
        service.printAllBuses();
    });

    EXPECT_NE(output.find("Автобусы не найдены"), std::string::npos);
}
