#pragma once
#include "database.h"
#include <string>

class ReportService {
private:
    Database& db;

public:
    explicit ReportService(Database& database);

    int getCrewIdByMemberId(int crewMemberId);

    void printBusSummary(const std::string& busNumber,
                         const std::string& startDate,
                         const std::string& endDate);

    void printCrewIncomeByPeriod(const std::string& startDate,
                                 const std::string& endDate);

    bool calculateCrewPayments(double percentRate,
                               const std::string& startDate,
                               const std::string& endDate);

    void printCrewPaymentOnDate(int crewId, const std::string& dateValue);
    void printMostExpensiveRouteInfo();
    void printTopMileageBusInfo();
    void printCrewMemberOwnInfo(int crewMemberId);

    bool saveBusImageFromFile(const std::string& busNumber, const std::string& filePath);
    bool exportBusImageToFile(const std::string& busNumber, const std::string& outputPath);
    void printBusImageInfo(const std::string& busNumber);
};