#pragma once
#include "database.h"
#include <string>

class RouteService {
private:
    Database& db;

public:
    explicit RouteService(Database& database);

    bool addRoute(const std::string& routeName,
                  const std::string& startPoint,
                  const std::string& endPoint,
                  double distanceKm);

    bool updateRoute(int routeId,
                     const std::string& routeName,
                     const std::string& startPoint,
                     const std::string& endPoint,
                     double distanceKm);

    bool deleteRoute(int routeId);

    void printAllRoutes();
};