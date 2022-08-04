#include <gtest/gtest.h>

#include <filesystem>
#include <fstream>

#include "cartridge.h"

#ifdef TEST_DIR
const std::filesystem::path test_dir{TEST_DIR};
#else
const std::filesystem::path test_dir{""};
#endif

namespace nesem {

// magic string "NES^Z"
constexpr std::array<char, 4> NES_TAG = {0x4E, 0x45, 0x53, 0x1A};

constexpr uint8_t INES_VER_1 = 0b0000;
constexpr uint8_t INES_VER_2 = 0b1000;

struct Input {
  uint8_t numRomBanks = 0;
  uint8_t numVromBanks = 0;
  uint8_t mapperType = 0;
  bool fourScreen = false;
  bool trainer = false;
  bool batteryRam = false;
  bool vertical = false;
  uint8_t inesVer = INES_VER_1;
  uint8_t prgRamUnits = 0;
  std::vector<uint8_t> prg;
  std::vector<uint8_t> chr;
};

void prepare(std::iostream* is, const Input& input) {
  // header
  for (auto c : NES_TAG) is->put(c);
  is->put(input.numRomBanks);
  is->put(input.numVromBanks);
  uint8_t controlByte1 = ((input.mapperType & 0b1111) << 4) |
                         (input.fourScreen << 3) | (input.trainer << 2) |
                         (input.batteryRam << 1) | (input.vertical);
  is->put(controlByte1);
  uint8_t controlByte2 = (input.mapperType & 0b11110000) | (input.inesVer);
  is->put(controlByte2);
  is->put(input.prgRamUnits);
  for (int i = 0; i < 7; ++i) is->put(0);

  if (input.trainer) {
    for (int i = 0; i < 512; ++i) is->put(0);
  }
  for (auto b : input.prg) is->put(b);
  for (auto b : input.chr) is->put(b);
}

TEST(InesTest, parse_simple_rom_dump) {
  std::stringstream ss;
  Input input = {};
  prepare(&ss, input);
  EXPECT_NO_THROW(load_ines_rom_dump(&ss));
}

TEST(InesTest, fail_empty) {
  std::stringstream ss;
  EXPECT_ANY_THROW(load_ines_rom_dump(&ss));
}

TEST(InesTest, fail_invalid_header) {
  std::stringstream ss;
  for (int i = 0; i < 16; ++i) ss << i;
  EXPECT_ANY_THROW(load_ines_rom_dump(&ss));
}

TEST(InesTest, fail_unsupported_mapper) {
  std::stringstream ss;
  Input input = {.mapperType = 0x01};
  prepare(&ss, input);
  EXPECT_ANY_THROW(load_ines_rom_dump(&ss));
}

TEST(InesTest, fail_ines_ver_2) {
  std::stringstream ss;
  Input input = {.inesVer = INES_VER_2};
  prepare(&ss, input);
  EXPECT_ANY_THROW(load_ines_rom_dump(&ss));
}

TEST(InesTest, vertical_mirroring) {
  std::stringstream ss;
  Input input = {.vertical = true};
  prepare(&ss, input);
  Cartridge c = load_ines_rom_dump(&ss);
  ASSERT_EQ(c.mirroring, ScreenMirroring::Vertical);
}

TEST(InesTest, fourscreen_mirroring) {
  std::stringstream ss;
  Input input = {.fourScreen = true};
  prepare(&ss, input);
  Cartridge c = load_ines_rom_dump(&ss);
  ASSERT_EQ(c.mirroring, ScreenMirroring::FourScreen);
}

TEST(InesTest, fail_read_prg) {
  std::stringstream ss;
  Input input = {.numRomBanks = 1, .prg = {0x10}};
  prepare(&ss, input);
  EXPECT_ANY_THROW(load_ines_rom_dump(&ss));
}

TEST(InesTest, fail_read_chr) {
  std::stringstream ss;
  Input input = {.numVromBanks = 1, .chr = {0x10}};
  prepare(&ss, input);
  EXPECT_ANY_THROW(load_ines_rom_dump(&ss));
}

TEST(InesTest, read_prg) {
  std::stringstream ss;
  std::vector<uint8_t> prg(2 * 16 * 1024, 0x10);
  Input input = {.numRomBanks = 2, .prg = prg};
  prepare(&ss, input);
  Cartridge c = load_ines_rom_dump(&ss);
  ASSERT_EQ(c.prg, prg);
}

TEST(InesTest, read_chr) {
  std::stringstream ss;
  std::vector<uint8_t> chr(2 * 8 * 1024, 0x10);
  Input input = {.numVromBanks = 2, .chr = chr};
  prepare(&ss, input);
  Cartridge c = load_ines_rom_dump(&ss);
  ASSERT_EQ(c.chr, chr);
}

TEST(InesTest, read_nestest_rom) {
  std::filesystem::path full_path = test_dir / "nestest.nes";
  std::fstream fs{full_path};
  ASSERT_NO_THROW(load_ines_rom_dump(&fs));
}

}  // namespace nesem
