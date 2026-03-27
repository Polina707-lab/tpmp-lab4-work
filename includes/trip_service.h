#pragma once
#include "database.h"
#include <string>

class TripService {
private:
    Database& db;

public:
    explicit TripService(Database& database);

    bool addTrip(int busId,
                 int routeId,
                 const std::string& departureDate,
                 const std::string& arrivalDate,
                 int passengerCount,
                 double ticketPrice);

    bool updateTrip(int tripId,
                    int busId,
                    int routeId,
                    const std::string& departureDate,
                    const std::string& arrivalDate,
                    int passengerCount,
                    double ticketPrice);

    bool deleteTrip(int tripId);

    void printTripsByBusAndPeriod(const std::string& busNumber,
                                  const std::string& startDate,
                                  const std::string& endDate);
};