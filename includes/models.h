#pragma once
#include <string>

struct UserSession {
    bool authenticated = false;
    int userId = -1;
    std::string login;
    std::string role;
    int crewMemberId = -1;
};

struct Bus {
    int busId = 0;
    std::string busNumber;
    std::string busName;
    double totalMileageKm = 0.0;
    std::string imagePath;
};

struct Route {
    int routeId = 0;
    std::string routeName;
    std::string startPoint;
    std::string endPoint;
    double distanceKm = 0.0;
};

struct Trip {
    int tripId = 0;
    int busId = 0;
    int routeId = 0;
    std::string departureDate;
    std::string arrivalDate;
    int passengerCount = 0;
    double ticketPrice = 0.0;
};