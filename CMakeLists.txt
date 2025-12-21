cmake_minimum_required(VERSION 3.5)
project(chat_service CXX)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
include(DownloadUserver)
set(CMAKE_CXX_STANDARD 23)

find_package(
    userver
    COMPONENTS core #
    postgresql
    QUIET
)
if(NOT userver_FOUND)
    # Tries TRY_DIR first, falls back to downloading userver from GitHub using CPM.
    download_userver(TRY_DIR third_party/userver GIT_TAG v2.13)
endif()

userver_setup_environment()

# third party
include(jwt)

# Common sources
file(GLOB_RECURSE SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/src/*.[hc]pp)
file(GLOB_RECURSE UNIT_TEST_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/src/*_test.cpp)
file(GLOB_RECURSE BENCHMARK_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/src/*_benchmark.cpp)
file(GLOB_RECURSE LIBUBENCH_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/src/benchmarks/main.cpp)
list(REMOVE_ITEM SOURCES ${BENCH_SOURCES} ${LIBUBENCH_SOURCES})
list(REMOVE_ITEM SOURCES ${UNIT_TEST_SOURCES} ${BENCHMARK_SOURCES} ${LIBUBENCH_SOURCES} ${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp)

add_library(${PROJECT_NAME}_objs OBJECT ${SOURCES})
include_directories(src)
target_link_libraries(
    ${PROJECT_NAME}_objs
    PUBLIC userver::core #
    userver::postgresql
)


# The Service
add_executable(${PROJECT_NAME} src/main.cpp)
target_link_libraries(${PROJECT_NAME} PRIVATE ${PROJECT_NAME}_objs)
target_link_libraries(${PROJECT_NAME}_objs PUBLIC jwt-cpp)

userver_add_sql_library(
    ${PROJECT_NAME}_sql
    NAMESPACE NChat
    OUTPUT_DIR ${CMAKE_CURRENT_BINARY_DIR}
    SQL_FILES src/infra/db/sql/*.sql
)
target_link_libraries(${PROJECT_NAME}_objs PUBLIC ${PROJECT_NAME}_sql)

# Unit Tests
add_executable(${PROJECT_NAME}_unittest ${UNIT_TEST_SOURCES})
target_link_libraries(${PROJECT_NAME}_unittest PRIVATE ${PROJECT_NAME}_objs userver::utest)
add_google_tests(${PROJECT_NAME}_unittest)

# Benchmarks
# add_executable(${PROJECT_NAME}_benchmark ${BENCHMARK_SOURCES})
# target_link_libraries(${PROJECT_NAME}_benchmark PRIVATE ${PROJECT_NAME}_objs userver::ubench)
# add_google_benchmark_tests(${PROJECT_NAME}_benchmark)

# Functional testing
userver_testsuite_add_simple(
    REQUIREMENTS ${CMAKE_CURRENT_SOURCE_DIR}/tests/requirements.txt
    SQL_LIBRARY ${PROJECT_NAME}_sql
)

# Install
include(GNUInstallDirs)

if(DEFINED ENV{PREFIX})
    message(STATUS "Set install prefix: $ENV{PREFIX}")
    file(TO_CMAKE_PATH "$ENV{PREFIX}" PREFIX_PATH)
    set(CMAKE_INSTALL_PREFIX "${PREFIX_PATH}")
endif()

file(GLOB CONFIGS_FILES ${CMAKE_CURRENT_SOURCE_DIR}/configs/*.yaml ${CMAKE_CURRENT_SOURCE_DIR}/configs/*.json)

install(
    TARGETS ${PROJECT_NAME}
    DESTINATION ${CMAKE_INSTALL_BINDIR}
    COMPONENT ${PROJECT_NAME}
)
install(
    FILES ${CONFIGS_FILES}
    DESTINATION ${CMAKE_INSTALL_SYSCONFDIR}/${PROJECT_NAME}
    COMPONENT ${PROJECT_NAME}
)
