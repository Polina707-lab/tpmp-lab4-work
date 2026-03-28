#include <gtest/gtest.h>
#include "database.h"
#include "auth.h"
#include <cstdio>

class AuthServiceTest : public ::testing::Test {
protected:
    Database db;

    void SetUp() override {
        std::remove(TEST_DB_PATH);
        ASSERT_TRUE(db.open(TEST_DB_PATH));
        ASSERT_TRUE(db.executeScriptFromFile(INIT_SQL_PATH));
    }

    void TearDown() override {
        db.close();
        std::remove(TEST_DB_PATH);
    }
};

TEST_F(AuthServiceTest, AdminLoginSuccess) {
    AuthService auth(db);
    UserSession session = auth.login("admin", "admin123");

    EXPECT_TRUE(session.authenticated);
    EXPECT_EQ(session.role, "admin");
    EXPECT_EQ(session.login, "admin");
    EXPECT_GT(session.userId, 0);
    EXPECT_EQ(session.crewMemberId, -1);
}

TEST_F(AuthServiceTest, CrewLoginSuccess) {
    AuthService auth(db);
    UserSession session = auth.login("ivanov", "1234");

    EXPECT_TRUE(session.authenticated);
    EXPECT_EQ(session.role, "crew");
    EXPECT_EQ(session.login, "ivanov");
    EXPECT_EQ(session.crewMemberId, 1);
    EXPECT_GT(session.userId, 0);
}

TEST_F(AuthServiceTest, CrewLoginHasValidUserId) {
    AuthService auth(db);
    UserSession session = auth.login("ivanov", "1234");

    EXPECT_TRUE(session.userId > 0);
}

TEST_F(AuthServiceTest, LoginFailsWithWrongPassword) {
    AuthService auth(db);
    UserSession session = auth.login("admin", "wrong_password");

    EXPECT_FALSE(session.authenticated);
    EXPECT_EQ(session.userId, -1);
    EXPECT_EQ(session.role, "");
    EXPECT_EQ(session.login, "");
    EXPECT_EQ(session.crewMemberId, -1);
}

TEST_F(AuthServiceTest, LoginFailsWithNonExistingUser) {
    AuthService auth(db);
    UserSession session = auth.login("nonexistent", "1234");

    EXPECT_FALSE(session.authenticated);
    EXPECT_EQ(session.userId, -1);
    EXPECT_EQ(session.role, "");
    EXPECT_EQ(session.login, "");
    EXPECT_EQ(session.crewMemberId, -1);
}

TEST_F(AuthServiceTest, FailedLoginHasDefaultValues) {
    AuthService auth(db);
    UserSession session = auth.login("wrong", "wrong");

    EXPECT_FALSE(session.authenticated);
    EXPECT_EQ(session.userId, -1);
    EXPECT_EQ(session.crewMemberId, -1);
    EXPECT_EQ(session.login, "");
    EXPECT_EQ(session.role, "");
}

TEST_F(AuthServiceTest, LoginFailsWithEmptyLogin) {
    AuthService auth(db);
    UserSession session = auth.login("", "1234");

    EXPECT_FALSE(session.authenticated);
    EXPECT_EQ(session.userId, -1);
    EXPECT_EQ(session.login, "");
    EXPECT_EQ(session.role, "");
    EXPECT_EQ(session.crewMemberId, -1);
}

TEST_F(AuthServiceTest, LoginFailsWithEmptyPassword) {
    AuthService auth(db);
    UserSession session = auth.login("admin", "");

    EXPECT_FALSE(session.authenticated);
    EXPECT_EQ(session.userId, -1);
    EXPECT_EQ(session.login, "");
    EXPECT_EQ(session.role, "");
    EXPECT_EQ(session.crewMemberId, -1);
}

TEST_F(AuthServiceTest, LoginFailsWithBothCredentialsEmpty) {
    AuthService auth(db);
    UserSession session = auth.login("", "");

    EXPECT_FALSE(session.authenticated);
    EXPECT_EQ(session.userId, -1);
    EXPECT_EQ(session.login, "");
    EXPECT_EQ(session.role, "");
    EXPECT_EQ(session.crewMemberId, -1);
}

TEST_F(AuthServiceTest, LoginIsCaseSensitive) {
    AuthService auth(db);
    UserSession session = auth.login("Admin", "admin123");

    EXPECT_FALSE(session.authenticated);
    EXPECT_EQ(session.userId, -1);
}

TEST_F(AuthServiceTest, CrewLoginReturnsCorrectLoginValue) {
    AuthService auth(db);
    UserSession session = auth.login("petrov", "1234");

    EXPECT_TRUE(session.authenticated);
    EXPECT_EQ(session.login, "petrov");
    EXPECT_EQ(session.role, "crew");
    EXPECT_EQ(session.crewMemberId, 2);
}
