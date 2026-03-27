#include "auth.h"
#include "bus_service.h"
#include "database.h"
#include "menu.h"
#include "report_service.h"
#include "route_service.h"
#include "trip_service.h"
#include <iostream>

int main() {
    Database db;

    if (!db.open("data/tourist_bureau.db")) {
        std::cerr << "Не удалось открыть БД.\n";
        return 1;
    }

    if (!db.isInitialized()) {
        std::cout << "База данных не инициализирована. Выполняется init.sql...\n";
        if (!db.executeScriptFromFile("sql/init.sql")) {
            std::cerr << "Ошибка инициализации БД.\n";
            return 1;
        }
    }

    AuthService authService(db);
    TripService tripService(db);
    ReportService reportService(db);
    BusService busService(db);
    RouteService routeService(db);

    Menu menu(authService, tripService, reportService, busService, routeService);
    menu.run();

    db.close();
    return 0;
}