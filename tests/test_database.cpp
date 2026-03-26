#include <gtest/gtest.h>
#include "database.h"

class DatabaseTest : public ::testing::Test {
protected:
    Database db;
};

TEST_F(DatabaseTest, OpenDatabaseSuccess) {
    EXPECT_TRUE(db.open("data/tourist_bureau.db"));
    db.close();
}

TEST_F(DatabaseTest, IsInitializedReturnsTrueForExistingDatabase) {
    ASSERT_TRUE(db.open("data/tourist_bureau.db"));
    EXPECT_TRUE(db.isInitialized());
    db.close();
}

TEST_F(DatabaseTest, ExecuteValidSqlWorks) {
    ASSERT_TRUE(db.open("data/tourist_bureau.db"));
    EXPECT_TRUE(db.execute("SELECT 1;"));
    db.close();
}