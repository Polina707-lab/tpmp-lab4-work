#include <gtest/gtest.h>
#include "database.h"
#include <cstdio>
#include <string>

class DatabaseTest : public ::testing::Test {
protected:
    Database db;

    void SetUp() override {
        std::remove(TEST_DB_PATH);
        ASSERT_TRUE(db.open(TEST_DB_PATH));
    }

    void TearDown() override {
        db.close();
        std::remove(TEST_DB_PATH);
    }
};

TEST_F(DatabaseTest, OpenDatabaseSuccess) {
    EXPECT_NE(db.get(), nullptr);
}

TEST_F(DatabaseTest, ExecuteValidSqlWorks) {
    EXPECT_TRUE(db.execute("CREATE TABLE test_table (id INTEGER PRIMARY KEY, name TEXT);"));
}

TEST_F(DatabaseTest, ExecuteInvalidSqlFails) {
    EXPECT_FALSE(db.execute("THIS IS INVALID SQL;"));
}

TEST_F(DatabaseTest, IsInitializedReturnsFalseBeforeInitScript) {
    EXPECT_FALSE(db.isInitialized());
}

TEST_F(DatabaseTest, IsInitializedReturnsTrueAfterInitScript) {
    ASSERT_TRUE(db.executeScriptFromFile(INIT_SQL_PATH));
    EXPECT_TRUE(db.isInitialized());
}

TEST_F(DatabaseTest, ExecuteScriptFromFileWorks) {
    EXPECT_TRUE(db.executeScriptFromFile(INIT_SQL_PATH));
}

TEST_F(DatabaseTest, ExecuteScriptFromFileFailsForMissingFile) {
    EXPECT_FALSE(db.executeScriptFromFile("sql/no_such_file.sql"));
}

TEST_F(DatabaseTest, InitScriptCreatesUsersTable) {
    ASSERT_TRUE(db.executeScriptFromFile(INIT_SQL_PATH));

    EXPECT_TRUE(db.execute(
        "INSERT INTO users (login, password, role, crew_member_id) "
        "VALUES ('temp_user', '1234', 'admin', NULL);"
    ));
}

TEST_F(DatabaseTest, CloseResetsDatabasePointer) {
    ASSERT_NE(db.get(), nullptr);

    db.close();

    EXPECT_EQ(db.get(), nullptr);
}

TEST(DatabaseStandaloneTest, GetLastErrorReturnsMessageForClosedDatabase) {
    Database db;
    EXPECT_EQ(db.getLastError(), "Database is not opened");
}

TEST(DatabaseStandaloneTest, OpenCreatesUsableDatabaseFile) {
    std::remove(TEST_DB_PATH);

    Database db;
    ASSERT_TRUE(db.open(TEST_DB_PATH));
    EXPECT_NE(db.get(), nullptr);

    db.close();
    std::remove(TEST_DB_PATH);
}
