cmake_minimum_required(VERSION 3.2)

project(nesem_test)

enable_testing()

find_package(GTest)

include_directories(${CMAKE_SOURCE_DIR}/src)

add_executable(runUnitTests instruction_set_test.cc)

target_link_libraries(runUnitTests GTest::gtest_main libnesem)
add_test(UnitTests runUnitTests)
