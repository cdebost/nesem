#include <fmt/core.h>

#include <filesystem>
#include <fstream>

#include "cartridge.h"
#include "cpu.h"
#include "mmu.h"
#include "nes.h"
#include "trace.h"

const std::filesystem::path kTestDir{TEST_DIR};
const std::filesystem::path kNestestPath = kTestDir / "nestest.nes";
const std::filesystem::path kExpectedPath = kTestDir / "nestest.log";
const std::filesystem::path kActualPath = kTestDir / "nestest_actual.log";

nesem::Cartridge load_nestest_cartridge() {
  if (!std::filesystem::exists(kNestestPath))
    throw std::runtime_error(
        fmt::format("File not found: {}", kNestestPath.string()));
  std::fstream fs{kNestestPath};
  try {
    return nesem::load_ines_rom_dump(&fs);
  } catch (const std::exception &e) {
    throw std::runtime_error(fmt::format("Failed to load ines file {}: {}\n",
                                         kNestestPath.string(), e.what()));
  }
}

void trace_nestest(std::ostream *output) {
  nesem::Cartridge cartridge = load_nestest_cartridge();
  nesem::Nes nes{cartridge};
  nes.reset();

  // Special state for this test
  nes.cpu.pc = 0xC000;
  nes.cpu.write(0x4004, 0xFF);
  nes.cpu.write(0x4005, 0xFF);
  nes.cpu.write(0x4006, 0xFF);
  nes.cpu.write(0x4007, 0xFF);
  nes.cpu.write(0x4015, 0xFF);

  for (int i = 0; i < 8991; ++i) {
    *output << nesem::trace_explain_state(nes) << "\n";
    nes.step();
  }
}

std::string rtrim(const std::string &str) {
  size_t end = str.find_last_not_of(" \n\r\t\f\v");
  return (end == std::string::npos) ? "" : str.substr(0, end + 1);
}

void verify_match(std::istream *expected, std::istream *actual) {
  expected->seekg(0, std::ios::beg);
  actual->seekg(0, std::ios::beg);

  int line = 0;
  while (!expected->eof()) {
    std::string expected_line;
    std::string actual_line;
    std::getline(*expected, expected_line);
    std::getline(*actual, actual_line);

    if (rtrim(expected_line) != rtrim(actual_line))
      throw std::runtime_error(fmt::format(
          "Mismatch at line {}:\n  Expected:\n    {}\n  Actual:\n    {}", line,
          expected_line, actual_line));

    ++line;
  }
}

int main() {
  try {
    std::fstream expected{kExpectedPath};
    std::fstream actual{kActualPath, std::fstream::out | std::fstream::in};

    fmt::print("Running {}\n", kNestestPath.string());
    trace_nestest(&actual);
    fmt::print("Comparing results with {}\n", kExpectedPath.string());
    verify_match(&expected, &actual);
    fmt::print("{}\n", "Output succesfully matched.");
  } catch (const std::exception &e) {
    fmt::print("{}\nActual results written to {}\n", e.what(),
               kActualPath.string());
    return 1;
  }
}
