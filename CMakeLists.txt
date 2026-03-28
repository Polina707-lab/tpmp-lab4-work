cmake_minimum_required(VERSION 3.14)
project(TouristBureau)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)

find_package(SQLite3 REQUIRED)

include_directories(includes)

option(ENABLE_COVERAGE "Enable code coverage" OFF)

if(ENABLE_COVERAGE)
    message(STATUS "Coverage enabled")
    add_compile_options(-O0 --coverage)
    add_link_options(--coverage)
endif()

add_executable(app
    src/main.cpp
    src/database.cpp
    src/auth.cpp
    src/trip_service.cpp
    src/report_service.cpp
    src/bus_service.cpp
    src/route_service.cpp
    src/menu.cpp
)

target_link_libraries(app SQLite::SQLite3)

option(BUILD_TESTS "Build tests" ON)
if(BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()
