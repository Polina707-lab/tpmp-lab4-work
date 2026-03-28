#include <gtest/gtest.h>
#include "database.h"
#include "route_service.h"
#include <sqlite3.h>
#include <string>
#include <cstdio>
#include <functional>

class RouteServiceTest : public ::testing::Test {
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

    int getRouteCount() {
        const char* sql = "SELECT COUNT(*) FROM routes;";
        sqlite3_stmt* stmt = nullptr;
        sqlite3_prepare_v2(db.get(), sql, -1, &stmt, nullptr);

        int count = 0;
        if (sqlite3_step(stmt) == SQLITE_ROW) {
            count = sqlite3_column_int(stmt, 0);
        }

        sqlite3_finalize(stmt);
        return count;
    }

    int getRouteIdByName(const std::string& routeName) {
        const char* sql = "SELECT route_id FROM routes WHERE route_name = ?;";
        sqlite3_stmt* stmt = nullptr;
        sqlite3_prepare_v2(db.get(), sql, -1, &stmt, nullptr);
        sqlite3_bind_text(stmt, 1, routeName.c_str(), -1, SQLITE_TRANSIENT);

        int routeId = -1;
        if (sqlite3_step(stmt) == SQLITE_ROW) {
            routeId = sqlite3_column_int(stmt, 0);
        }

        sqlite3_finalize(stmt);
        return routeId;
    }

    std::string captureOutput(const std::function<void()>& func) {
        testing::internal::CaptureStdout();
        func();
        return testing::internal::GetCapturedStdout();
    }
};

TEST_F(RouteServiceTest, AddRouteSuccess) {
    RouteService service(db);

    int before = getRouteCount();

    bool result = service.addRoute("Тестовый маршрут", "Пункт А", "Пункт Б", 111.0);
    EXPECT_TRUE(result);

    int after = getRouteCount();
    EXPECT_EQ(after, before + 1);
}

TEST_F(RouteServiceTest, AddRouteFailsWithDuplicateName) {
    RouteService service(db);

    bool result = service.addRoute("Золотое кольцо", "Новый пункт", "Другой пункт", 123.0);
    EXPECT_FALSE(result);
}

TEST_F(RouteServiceTest, AddRouteFailsWithZeroDistance) {
    RouteService service(db);

    bool result = service.addRoute("Нулевой маршрут", "Пункт А", "Пункт Б", 0.0);
    EXPECT_FALSE(result);
}

TEST_F(RouteServiceTest, AddRouteFailsWithNegativeDistance) {
    RouteService service(db);

    bool result = service.addRoute("Отрицательный маршрут", "Пункт А", "Пункт Б", -10.0);
    EXPECT_FALSE(result);
}

TEST_F(RouteServiceTest, UpdateRouteSuccess) {
    RouteService service(db);

    ASSERT_TRUE(service.addRoute("Маршрут для обновления", "A", "B", 50.0));
    int routeId = getRouteIdByName("Маршрут для обновления");
    ASSERT_NE(routeId, -1);

    bool result = service.updateRoute(routeId, "Обновленный маршрут", "C", "D", 75.0);
    EXPECT_TRUE(result);
}

TEST_F(RouteServiceTest, UpdateRouteChangesStoredData) {
    RouteService service(db);

    ASSERT_TRUE(service.addRoute("Маршрут для проверки", "A", "B", 60.0));
    int routeId = getRouteIdByName("Маршрут для проверки");
    ASSERT_NE(routeId, -1);

    ASSERT_TRUE(service.updateRoute(routeId, "Новый маршрут", "C", "D", 80.0));

    const char* sql =
        "SELECT route_name, start_point, end_point, distance_km "
        "FROM routes WHERE route_id = ?;";
    sqlite3_stmt* stmt = nullptr;
    sqlite3_prepare_v2(db.get(), sql, -1, &stmt, nullptr);
    sqlite3_bind_int(stmt, 1, routeId);

    ASSERT_EQ(sqlite3_step(stmt), SQLITE_ROW);

    std::string routeName = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
    std::string startPoint = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
    std::string endPoint = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
    double distanceKm = sqlite3_column_double(stmt, 3);

    sqlite3_finalize(stmt);

    EXPECT_EQ(routeName, "Новый маршрут");
    EXPECT_EQ(startPoint, "C");
    EXPECT_EQ(endPoint, "D");
    EXPECT_EQ(distanceKm, 80.0);
}

TEST_F(RouteServiceTest, UpdateRouteFailsForInvalidId) {
    RouteService service(db);

    bool result = service.updateRoute(9999, "Несуществующий маршрут", "A", "B", 10.0);
    EXPECT_FALSE(result);
}

TEST_F(RouteServiceTest, UpdateRouteFailsWithDuplicateName) {
    RouteService service(db);

    ASSERT_TRUE(service.addRoute("Уникальный маршрут", "A", "B", 50.0));
    int routeId = getRouteIdByName("Уникальный маршрут");
    ASSERT_NE(routeId, -1);

    bool result = service.updateRoute(routeId, "Золотое кольцо", "C", "D", 70.0);
    EXPECT_FALSE(result);
}

TEST_F(RouteServiceTest, DeleteRouteSuccess) {
    RouteService service(db);

    ASSERT_TRUE(service.addRoute("Маршрут для удаления", "X", "Y", 90.0));
    int routeId = getRouteIdByName("Маршрут для удаления");
    ASSERT_NE(routeId, -1);

    EXPECT_TRUE(service.deleteRoute(routeId));
}

TEST_F(RouteServiceTest, DeleteRouteRemovesRecordFromDatabase) {
    RouteService service(db);

    ASSERT_TRUE(service.addRoute("Маршрут исчезнет", "X", "Y", 95.0));
    int routeId = getRouteIdByName("Маршрут исчезнет");
    ASSERT_NE(routeId, -1);

    ASSERT_TRUE(service.deleteRoute(routeId));

    EXPECT_EQ(getRouteIdByName("Маршрут исчезнет"), -1);
}

TEST_F(RouteServiceTest, DeleteRouteFailsForInvalidId) {
    RouteService service(db);

    EXPECT_FALSE(service.deleteRoute(9999));
}

TEST_F(RouteServiceTest, PrintAllRoutesShowsExistingRoutes) {
    RouteService service(db);

    std::string output = captureOutput([&]() {
        service.printAllRoutes();
    });

    EXPECT_NE(output.find("Список маршрутов"), std::string::npos);
    EXPECT_NE(output.find("Золотое кольцо"), std::string::npos);
}

TEST_F(RouteServiceTest, PrintAllRoutesShowsMessageWhenNoRoutesExist) {
    ASSERT_TRUE(db.execute("DELETE FROM trips;"));
    ASSERT_TRUE(db.execute("DELETE FROM routes;"));

    RouteService service(db);

    std::string output = captureOutput([&]() {
        service.printAllRoutes();
    });

    EXPECT_NE(output.find("Маршруты не найдены"), std::string::npos);
}
