cmake_minimum_required(VERSION 3.2)

project(nesem_test)

enable_testing()

find_package(GTest)

add_executable(runUnitTests test.cc)

target_link_libraries(runUnitTests GTest::gtest_main)
add_test(UnitTests runUnitTests)
