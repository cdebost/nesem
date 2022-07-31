project(nesem_src)

include_directories(${PROJECT_SOURCE_DIR})

add_library(libnesem instruction_set.cc assembler/assembler.cc assembler/scanner.cc assembler/parser.cc)
set_target_properties(libnesem PROPERTIES PREFIX "")

add_executable(nesem main.cc)
target_link_libraries(nesem libnesem)
