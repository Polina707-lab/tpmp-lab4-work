#include <gtest/gtest.h>
#include "database.h"
#include "auth.h"
#include <cstdio>

class AuthServiceTest : public ::testing::Test {
protected:
    Database db;

    void SetUp() override {
        ASSERT_TRUE(db.open("data/test.db"));
        ASSERT_TRUE(db.executeScriptFromFile("sql/init.sql"));
    }

    void TearDown() override {
        db.close();
        std::remove("data/test.db");
    }
};

TEST_F(AuthServiceTest, AdminLoginSuccess) {
    AuthService auth(db);
    UserSession session = auth.login("admin", "admin123");

    EXPECT_TRUE(session.authenticated);
    EXPECT_EQ(session.role, "admin");
}

TEST_F(AuthServiceTest, CrewLoginSuccess) {
    AuthService auth(db);
    UserSession session = auth.login("ivanov", "1234");

    EXPECT_TRUE(session.authenticated);
    EXPECT_EQ(session.role, "crew");
    EXPECT_EQ(session.crewMemberId, 1);
}

TEST_F(AuthServiceTest, LoginFailsWithWrongPassword) {
    AuthService auth(db);
    UserSession session = auth.login("admin", "wrong_password");

    EXPECT_FALSE(session.authenticated);
}

TEST_F(AuthServiceTest, LoginFailsWithNonExistingUser) {
    AuthService auth(db);
    UserSession session = auth.login("nonexistent", "1234");

    EXPECT_FALSE(session.authenticated);
}

TEST_F(AuthServiceTest, CrewLoginHasValidUserId) {
    AuthService auth(db);
    UserSession session = auth.login("ivanov", "1234");

    EXPECT_TRUE(session.userId > 0);
}

TEST_F(AuthServiceTest, FailedLoginHasDefaultValues) {
    AuthService auth(db);
    UserSession session = auth.login("wrong", "wrong");

    EXPECT_FALSE(session.authenticated);
    EXPECT_EQ(session.userId, -1);
    EXPECT_EQ(session.crewMemberId, -1);
}