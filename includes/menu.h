#pragma once
#include "auth.h"
#include "bus_service.h"
#include "report_service.h"
#include "route_service.h"
#include "trip_service.h"

class Menu {
private:
    AuthService& authService;
    TripService& tripService;
    ReportService& reportService;
    BusService& busService;
    RouteService& routeService;

    void adminMenu(const UserSession& session);
    void crewMenu(const UserSession& session);

public:
    Menu(AuthService& auth,
         TripService& trip,
         ReportService& report,
         BusService& bus,
         RouteService& route);

    void run();
};