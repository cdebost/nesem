project(nesem_src)

include_directories(${PROJECT_SOURCE_DIR})

add_library(libnesem instruction_set.cc assembler/assembler.cc assembler/scanner.cc assembler/parser.cc cpu.cc cartridge.cc mmu.cc trace.cc ppu.cc render.cc)
set_target_properties(libnesem PROPERTIES PREFIX "")
target_link_libraries(libnesem CONAN_PKG::fmt)
target_link_libraries(libnesem CONAN_PKG::sdl)

add_executable(nesem main.cc)
target_link_libraries(nesem libnesem)
