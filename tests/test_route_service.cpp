#include <gtest/gtest.h>
#include "database.h"
#include "route_service.h"
#include <sqlite3.h>
#include <string>
#include <cstdio>

class RouteServiceTest : public ::testing::Test {
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

TEST_F(RouteServiceTest, UpdateRouteSuccess) {
    RouteService service(db);

    ASSERT_TRUE(service.addRoute("Маршрут для обновления", "A", "B", 50.0));

    const char* sql = "SELECT route_id FROM routes WHERE route_name = 'Маршрут для обновления';";
    sqlite3_stmt* stmt = nullptr;
    sqlite3_prepare_v2(db.get(), sql, -1, &stmt, nullptr);

    int routeId = -1;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        routeId = sqlite3_column_int(stmt, 0);
    }

    sqlite3_finalize(stmt);
    ASSERT_NE(routeId, -1);

    bool result = service.updateRoute(routeId, "Обновленный маршрут", "C", "D", 75.0);
    EXPECT_TRUE(result);
}

TEST_F(RouteServiceTest, UpdateRouteFailsForInvalidId) {
    RouteService service(db);

    bool result = service.updateRoute(9999, "Несуществующий маршрут", "A", "B", 10.0);
    EXPECT_FALSE(result);
}

TEST_F(RouteServiceTest, DeleteRouteSuccess) {
    RouteService service(db);

    ASSERT_TRUE(service.addRoute("Маршрут для удаления", "X", "Y", 90.0));

    const char* sql = "SELECT route_id FROM routes WHERE route_name = 'Маршрут для удаления';";
    sqlite3_stmt* stmt = nullptr;
    sqlite3_prepare_v2(db.get(), sql, -1, &stmt, nullptr);

    int routeId = -1;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        routeId = sqlite3_column_int(stmt, 0);
    }

    sqlite3_finalize(stmt);
    ASSERT_NE(routeId, -1);

    EXPECT_TRUE(service.deleteRoute(routeId));
}

TEST_F(RouteServiceTest, DeleteRouteFailsForInvalidId) {
    RouteService service(db);

    EXPECT_FALSE(service.deleteRoute(9999));
}