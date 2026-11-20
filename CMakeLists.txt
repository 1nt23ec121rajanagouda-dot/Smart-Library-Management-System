cmake_minimum_required(VERSION 3.16)
project(SmartLibraryManagement VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Compiler warnings
add_compile_options(-Wall -Wextra -Wpedantic)

# ── Gather sources ──────────────────────────────────────────────────────────
file(GLOB_RECURSE LIB_SOURCES
    "${CMAKE_SOURCE_DIR}/src/utils/*.cpp"
    "${CMAKE_SOURCE_DIR}/src/repositories/*.cpp"
    "${CMAKE_SOURCE_DIR}/src/services/*.cpp"
    "${CMAKE_SOURCE_DIR}/src/cli/*.cpp"
)

# ── Core library (everything except main) ───────────────────────────────────
add_library(library_core STATIC ${LIB_SOURCES})
target_include_directories(library_core PUBLIC "${CMAKE_SOURCE_DIR}/src")

# macOS CommonCrypto for SHA-256 (part of libSystem, no extra framework needed)
if(APPLE)
    # CommonCrypto is part of libSystem on macOS; typically no explicit link needed.
    # If linker complains, uncomment:
    # target_link_libraries(library_core PUBLIC "-framework Security")
endif()

# ── Optional MySQL Connector/C++ ───────────────────────────────────────────
find_package(PkgConfig QUIET)
if(PkgConfig_FOUND)
    pkg_check_modules(MYSQLCPPCONN QUIET mysqlcppconn)
endif()

if(MYSQLCPPCONN_FOUND)
    message(STATUS "MySQL Connector/C++ found — MySQL storage enabled")
    target_compile_definitions(library_core PUBLIC HAS_MYSQL=1)
    target_include_directories(library_core PUBLIC ${MYSQLCPPCONN_INCLUDE_DIRS})
    target_link_libraries(library_core PUBLIC ${MYSQLCPPCONN_LIBRARIES})
else()
    message(STATUS "MySQL Connector/C++ NOT found — using file-backed storage only")
endif()

# ── Main executable ────────────────────────────────────────────────────────
add_executable(library_app src/main.cpp)
target_link_libraries(library_app PRIVATE library_core)

# ── Test executable ────────────────────────────────────────────────────────
file(GLOB_RECURSE TEST_SOURCES "${CMAKE_SOURCE_DIR}/tests/*.cpp")
if(TEST_SOURCES)
    add_executable(library_tests ${TEST_SOURCES})
    target_link_libraries(library_tests PRIVATE library_core)
    enable_testing()
    add_test(NAME LibraryTests COMMAND library_tests)
endif()
