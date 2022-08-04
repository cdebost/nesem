cmake_minimum_required(VERSION 3.2)

project(nesem_test)

enable_testing()

find_package(GTest)

include_directories(${CMAKE_SOURCE_DIR}/src)

add_executable(runUnitTests instruction_set_test.cc assembler_test.cc cpu_test.cc ines_test.cc mmu_test.cc)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/nestest.nes ${CMAKE_CURRENT_BINARY_DIR} COPYONLY)
target_compile_definitions(runUnitTests PRIVATE TEST_DIR=\"${CMAKE_CURRENT_BINARY_DIR}\")

target_link_libraries(runUnitTests GTest::gtest_main libnesem)
add_test(UnitTests runUnitTests)
