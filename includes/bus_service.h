#pragma once
#include "database.h"
#include <string>

class BusService {
private:
    Database& db;

public:
    explicit BusService(Database& database);

    bool addBus(const std::string& busNumber,
                const std::string& busName,
                double totalMileageKm);

    bool updateBus(int busId,
                   const std::string& busNumber,
                   const std::string& busName,
                   double totalMileageKm);

    bool deleteBus(int busId);

    void printAllBuses();
};