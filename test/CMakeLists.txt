cmake_minimum_required(VERSION 3.2)

project(nesem_test)

find_package(GTest)

include_directories(${CMAKE_SOURCE_DIR}/src)

# unit tests

add_executable(unittests instruction_set_test.cc assembler_test.cc cpu_test.cc ines_test.cc mmu_test.cc trace_test.cc)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/nestest.nes ${CMAKE_CURRENT_BINARY_DIR} COPYONLY)
target_compile_definitions(unittests PRIVATE TEST_DIR=\"${CMAKE_CURRENT_BINARY_DIR}\")

target_link_libraries(unittests GTest::gtest_main libnesem)
add_test(UnitTests ${CMAKE_BINARY_DIR}/bin/unittests)

# nes test

add_executable(nestest nestest.cc)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/nestest.nes ${CMAKE_CURRENT_BINARY_DIR} COPYONLY)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/nestest.log ${CMAKE_CURRENT_BINARY_DIR} COPYONLY)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/nestest_noppu.log ${CMAKE_CURRENT_BINARY_DIR} COPYONLY)
target_compile_definitions(nestest PRIVATE TEST_DIR=\"${CMAKE_CURRENT_BINARY_DIR}\")

target_link_libraries(nestest libnesem)
add_test(NesTest ${CMAKE_BINARY_DIR}/bin/nestest)
