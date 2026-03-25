cmake_minimum_required(VERSION 3.14)
project(TouristBureau)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# куда складывать бинарник
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)

find_package(SQLite3 REQUIRED)

include_directories(includes)

add_executable(app
    src/main.cpp
)

target_link_libraries(app SQLite::SQLite3)