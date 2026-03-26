#include <gtest/gtest.h>
#include "database.h"
#include "report_service.h"
#include <sqlite3.h>
#include <string>
#include <fstream>
#include <cstdio>

class ReportServiceTest : public ::testing::Test {
protected:
    Database db;

    void SetUp() override {
        ASSERT_TRUE(db.open("data/test.db"));
        ASSERT_TRUE(db.executeScriptFromFile("sql/init.sql"));
    }

    void TearDown() override {
        db.close();
        std::remove("data/test.db");
        std::remove("data/test_blob.bin");
        std::remove("data/exported_test_blob.bin");
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

TEST_F(ReportServiceTest, SaveBusImageFromFileWorks) {
    ReportService report(db);

    const std::string testFile = "data/test_blob.bin";

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

TEST_F(ReportServiceTest, ExportBusImageToFileWorks) {
    ReportService report(db);

    const std::string inputFile = "data/test_blob.bin";
    const std::string outputFile = "data/exported_test_blob.bin";

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
