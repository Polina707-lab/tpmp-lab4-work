#include <gtest/gtest.h>
#include "database.h"
#include "report_service.h"
#include <sqlite3.h>
#include <string>
#include <fstream>
#include <sstream>
#include <cstdio>

class ReportServiceTest : public ::testing::Test {
protected:
    Database db;

    void SetUp() override {
        std::remove(TEST_DB_PATH);
        std::remove(TEST_BLOB_PATH);
        std::remove(TEST_EXPORT_BLOB_PATH);

        ASSERT_TRUE(db.open(TEST_DB_PATH));
        ASSERT_TRUE(db.executeScriptFromFile(INIT_SQL_PATH));
    }

    void TearDown() override {
        db.close();
        std::remove(TEST_DB_PATH);
        std::remove(TEST_BLOB_PATH);
        std::remove(TEST_EXPORT_BLOB_PATH);
    }

    std::string captureOutput(const std::function<void()>& func) {
        testing::internal::CaptureStdout();
        func();
        return testing::internal::GetCapturedStdout();
    }
};

TEST_F(ReportServiceTest, GetCrewIdByMemberIdWorks) {
    ReportService report(db);

    int crewId = report.getCrewIdByMemberId(1);
    EXPECT_EQ(crewId, 1);
}

TEST_F(ReportServiceTest, GetCrewIdByInvalidMemberIdReturnsMinusOne) {
    ReportService report(db);

    int crewId = report.getCrewIdByMemberId(999);
    EXPECT_EQ(crewId, -1);
}

TEST_F(ReportServiceTest, CalculateCrewPaymentsWorks) {
    ReportService report(db);

    bool result = report.calculateCrewPayments(20.0, "2024-01-01", "2024-12-31");
    EXPECT_TRUE(result);

    const char* sql =
        "SELECT COUNT(*) FROM crew_payments "
        "WHERE period_start = '2024-01-01' AND period_end = '2024-12-31' AND percent_rate = 20.0;";

    sqlite3_stmt* stmt = nullptr;
    sqlite3_prepare_v2(db.get(), sql, -1, &stmt, nullptr);

    int count = 0;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        count = sqlite3_column_int(stmt, 0);
    }

    sqlite3_finalize(stmt);

    EXPECT_GT(count, 0);
}

TEST_F(ReportServiceTest, CalculateCrewPaymentsCreatesThreeRows) {
    ReportService report(db);

    ASSERT_TRUE(report.calculateCrewPayments(20.0, "2024-01-01", "2024-12-31"));

    const char* sql =
        "SELECT COUNT(*) FROM crew_payments "
        "WHERE period_start = '2024-01-01' AND period_end = '2024-12-31' AND percent_rate = 20.0;";

    sqlite3_stmt* stmt = nullptr;
    sqlite3_prepare_v2(db.get(), sql, -1, &stmt, nullptr);

    int count = 0;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        count = sqlite3_column_int(stmt, 0);
    }

    sqlite3_finalize(stmt);

    EXPECT_EQ(count, 3);
}

TEST_F(ReportServiceTest, CalculateCrewPaymentsReplacesOldRowsForSamePeriodAndRate) {
    ReportService report(db);

    ASSERT_TRUE(report.calculateCrewPayments(20.0, "2024-01-01", "2024-12-31"));
    ASSERT_TRUE(report.calculateCrewPayments(20.0, "2024-01-01", "2024-12-31"));

    const char* sql =
        "SELECT COUNT(*) FROM crew_payments "
        "WHERE period_start = '2024-01-01' AND period_end = '2024-12-31' AND percent_rate = 20.0;";

    sqlite3_stmt* stmt = nullptr;
    sqlite3_prepare_v2(db.get(), sql, -1, &stmt, nullptr);

    int count = 0;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        count = sqlite3_column_int(stmt, 0);
    }

    sqlite3_finalize(stmt);

    EXPECT_EQ(count, 3);
}

TEST_F(ReportServiceTest, SaveBusImageFromFileWorks) {
    ReportService report(db);

    const std::string testFile = TEST_BLOB_PATH;

    {
        std::ofstream out(testFile, std::ios::binary);
        ASSERT_TRUE(out.is_open());
        out << "testimagecontent";
    }

    ASSERT_TRUE(report.saveBusImageFromFile("A101", testFile));

    const char* sql = "SELECT LENGTH(image_data) FROM buses WHERE bus_number = 'A101';";
    sqlite3_stmt* stmt = nullptr;
    sqlite3_prepare_v2(db.get(), sql, -1, &stmt, nullptr);

    int blobSize = 0;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        blobSize = sqlite3_column_int(stmt, 0);
    }

    sqlite3_finalize(stmt);

    EXPECT_GT(blobSize, 0);
}

TEST_F(ReportServiceTest, SaveBusImageFromFileFailsForMissingFile) {
    ReportService report(db);

    EXPECT_FALSE(report.saveBusImageFromFile("A101", "data/no_such_file.bin"));
}

TEST_F(ReportServiceTest, SaveBusImageFromFileFailsForInvalidBusNumber) {
    ReportService report(db);

    const std::string testFile = TEST_BLOB_PATH;

    {
        std::ofstream out(testFile, std::ios::binary);
        ASSERT_TRUE(out.is_open());
        out << "testimagecontent";
    }

    EXPECT_FALSE(report.saveBusImageFromFile("ZZ999", testFile));
}

TEST_F(ReportServiceTest, ExportBusImageToFileWorks) {
    ReportService report(db);

    const std::string inputFile = TEST_BLOB_PATH;
    const std::string outputFile = TEST_EXPORT_BLOB_PATH;

    {
        std::ofstream out(inputFile, std::ios::binary);
        ASSERT_TRUE(out.is_open());
        out << "testimagecontent";
    }

    ASSERT_TRUE(report.saveBusImageFromFile("A101", inputFile));
    ASSERT_TRUE(report.exportBusImageToFile("A101", outputFile));

    std::ifstream in(outputFile, std::ios::binary);
    EXPECT_TRUE(in.is_open());
}

TEST_F(ReportServiceTest, ExportBusImageToFileFailsWhenBusDoesNotExist) {
    ReportService report(db);

    EXPECT_FALSE(report.exportBusImageToFile("ZZ999", TEST_EXPORT_BLOB_PATH));
}

TEST_F(ReportServiceTest, ExportBusImageToFileFailsWhenImageMissing) {
    ReportService report(db);

    EXPECT_FALSE(report.exportBusImageToFile("B202", TEST_EXPORT_BLOB_PATH));
}

TEST_F(ReportServiceTest, PrintBusSummaryShowsStatisticsForExistingBus) {
    ReportService report(db);

    std::string output = captureOutput([&]() {
        report.printBusSummary("A101", "2024-01-01", "2024-12-31");
    });

    EXPECT_NE(output.find("Статистика по автобусу A101"), std::string::npos);
    EXPECT_NE(output.find("Количество поездок"), std::string::npos);
    EXPECT_NE(output.find("Количество пассажиров"), std::string::npos);
}

TEST_F(ReportServiceTest, PrintBusSummaryShowsNotFoundForUnknownBus) {
    ReportService report(db);

    std::string output = captureOutput([&]() {
        report.printBusSummary("ZZ999", "2024-01-01", "2024-12-31");
    });

    EXPECT_NE(output.find("Данные не найдены"), std::string::npos);
}

TEST_F(ReportServiceTest, PrintCrewPaymentOnDateShowsResultsAfterCalculation) {
    ReportService report(db);

    ASSERT_TRUE(report.calculateCrewPayments(20.0, "2024-01-01", "2024-12-31"));

    std::string output = captureOutput([&]() {
        report.printCrewPaymentOnDate(1, "2024-06-01");
    });

    EXPECT_NE(output.find("Начисления экипажу 1"), std::string::npos);
    EXPECT_NE(output.find("сумма"), std::string::npos);
}

TEST_F(ReportServiceTest, PrintCrewPaymentOnDateShowsNotFoundForMissingPeriod) {
    ReportService report(db);

    ASSERT_TRUE(report.calculateCrewPayments(20.0, "2024-01-01", "2024-12-31"));

    std::string output = captureOutput([&]() {
        report.printCrewPaymentOnDate(1, "2025-01-01");
    });

    EXPECT_NE(output.find("Начисления не найдены"), std::string::npos);
}

TEST_F(ReportServiceTest, PrintCrewMemberOwnInfoShowsMemberData) {
    ReportService report(db);

    std::string output = captureOutput([&]() {
        report.printCrewMemberOwnInfo(1);
    });

    EXPECT_NE(output.find("Ваши данные"), std::string::npos);
    EXPECT_NE(output.find("Иванов"), std::string::npos);
}

TEST_F(ReportServiceTest, PrintCrewMemberOwnInfoShowsNotFoundForInvalidMember) {
    ReportService report(db);

    std::string output = captureOutput([&]() {
        report.printCrewMemberOwnInfo(999);
    });

    EXPECT_NE(output.find("Информация о пользователе не найдена"), std::string::npos);
}

TEST_F(ReportServiceTest, PrintBusImageInfoShowsMissingImage) {
    ReportService report(db);

    std::string output = captureOutput([&]() {
        report.printBusImageInfo("A101");
    });

    EXPECT_NE(output.find("Информация об изображении автобуса"), std::string::npos);
    EXPECT_NE(output.find("отсутствует"), std::string::npos);
}

TEST_F(ReportServiceTest, PrintBusImageInfoShowsNotFoundForUnknownBus) {
    ReportService report(db);

    std::string output = captureOutput([&]() {
        report.printBusImageInfo("ZZ999");
    });

    EXPECT_NE(output.find("Автобус не найден"), std::string::npos);
}

TEST_F(ReportServiceTest, PrintMostExpensiveRouteInfoPrintsData) {
    ReportService report(db);

    std::string output = captureOutput([&]() {
        report.printMostExpensiveRouteInfo();
    });

    EXPECT_NE(output.find("Сведения по наиболее дорогому маршруту"), std::string::npos);
    EXPECT_NE(output.find("цена билета"), std::string::npos);
}

TEST_F(ReportServiceTest, PrintTopMileageBusInfoPrintsData) {
    ReportService report(db);

    std::string output = captureOutput([&]() {
        report.printTopMileageBusInfo();
    });

    EXPECT_NE(output.find("Автобус с наибольшим суммарным пробегом"), std::string::npos);
    EXPECT_NE(output.find("перевезено пассажиров"), std::string::npos);
}

TEST_F(ReportServiceTest, PrintCrewIncomeByPeriodPrintsData) {
    ReportService report(db);

    std::string output = captureOutput([&]() {
        report.printCrewIncomeByPeriod("2024-01-01", "2024-12-31");
    });

    EXPECT_NE(output.find("Начисления по каждому экипажу за период"), std::string::npos);
    EXPECT_NE(output.find("Экипаж ID"), std::string::npos);
}
