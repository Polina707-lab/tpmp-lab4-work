#include <gtest/gtest.h>
#include "database.h"
#include <cstdio>

class DatabaseTest : public ::testing::Test {
protected:
    Database db;

    void SetUp() override {
        ASSERT_TRUE(db.open("data/test.db"));
    }

    void TearDown() override {
        db.close();
        std::remove("data/test.db");
    }
};

TEST_F(DatabaseTest, OpenDatabaseSuccess) {
    EXPECT_TRUE(db.get() != nullptr);
}

TEST_F(DatabaseTest, ExecuteValidSqlWorks) {
    EXPECT_TRUE(db.execute("CREATE TABLE test_table (id INTEGER PRIMARY KEY, name TEXT);"));
}

TEST_F(DatabaseTest, IsInitializedReturnsTrueAfterInitScript) {
    ASSERT_TRUE(db.executeScriptFromFile("sql/init.sql"));
    EXPECT_TRUE(db.isInitialized());
}

TEST_F(DatabaseTest, ExecuteInvalidSqlFails) {
    EXPECT_FALSE(db.execute("THIS IS INVALID SQL;"));
}

TEST_F(DatabaseTest, ExecuteScriptFromFileWorks) {
    EXPECT_TRUE(db.executeScriptFromFile("sql/init.sql"));
}