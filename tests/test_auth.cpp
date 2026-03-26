#include <gtest/gtest.h>
#include "database.h"
#include "auth.h"

class AuthServiceTest : public ::testing::Test {
protected:
    Database db;

    void SetUp() override {
        ASSERT_TRUE(db.open("data/tourist_bureau.db"));
    }

    void TearDown() override {
        db.close();
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