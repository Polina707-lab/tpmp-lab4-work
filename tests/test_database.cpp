#include <gtest/gtest.h>
#include "database.h"
#include <cstdio>

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
    EXPECT_TRUE(db.get() != nullptr);
}

TEST_F(DatabaseTest, ExecuteValidSqlWorks) {
    EXPECT_TRUE(db.execute("CREATE TABLE test_table (id INTEGER PRIMARY KEY, name TEXT);"));
}

TEST_F(DatabaseTest, IsInitializedReturnsTrueAfterInitScript) {
    ASSERT_TRUE(db.executeScriptFromFile(INIT_SQL_PATH));
    EXPECT_TRUE(db.isInitialized());
}

TEST_F(DatabaseTest, ExecuteInvalidSqlFails) {
    EXPECT_FALSE(db.execute("THIS IS INVALID SQL;"));
}

TEST_F(DatabaseTest, ExecuteScriptFromFileWorks) {
    EXPECT_TRUE(db.executeScriptFromFile(INIT_SQL_PATH));
}