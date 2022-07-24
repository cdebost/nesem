project(nesem_src)

add_library(libnesem instruction_set.cc)
set_target_properties(libnesem PROPERTIES PREFIX "")

add_executable(nesem main.cc)
target_link_libraries(nesem libnesem)
