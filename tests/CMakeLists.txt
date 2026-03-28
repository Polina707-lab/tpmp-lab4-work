find_package(GTest REQUIRED)

include_directories(${GTEST_INCLUDE_DIRS} ../includes)

add_executable(runTests
    test_database.cpp
    test_auth.cpp
    test_trip_service.cpp
    test_report_service.cpp
    test_bus_service.cpp
    test_route_service.cpp
    ../src/database.cpp
    ../src/auth.cpp
    ../src/trip_service.cpp
    ../src/report_service.cpp
    ../src/bus_service.cpp
    ../src/route_service.cpp
)

target_compile_definitions(runTests PRIVATE
    TEST_DB_PATH="${CMAKE_SOURCE_DIR}/data/test.db"
    INIT_SQL_PATH="${CMAKE_SOURCE_DIR}/sql/init.sql"
    TEST_BLOB_PATH="${CMAKE_SOURCE_DIR}/data/test_blob.bin"
    TEST_EXPORT_BLOB_PATH="${CMAKE_SOURCE_DIR}/data/exported_test_blob.bin"
)

target_link_libraries(runTests
    GTest::GTest
    GTest::Main
    SQLite::SQLite3
)

add_test(NAME AllTests COMMAND runTests)
