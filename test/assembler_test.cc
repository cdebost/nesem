#include "assembler/assembler.h"

#include <gtest/gtest.h>

namespace nesem {
namespace assembler {

TEST(AssemblerTest, assemble_nothing) {
  auto out = assemble("");
  ASSERT_EQ(out.size(), 0);
}

TEST(AssemblerTest, assemble_blank) {
  auto out = assemble(" \n ");
  ASSERT_EQ(out.size(), 0);
}

TEST(AssemblerTest, assemble_implicit_instruction) {
  auto out = assemble("BRK");
  ASSERT_EQ(out, std::vector<uint8_t>{0x00});
}

TEST(AssemblerTest, assemble_immediate_instruction) {
  auto out = assemble("LDA #$05");
  ASSERT_EQ(out, (std::vector<uint8_t>{0xA9, 0x05}));
}

TEST(AssemblerTest, assemble_zeropage_instruction) {
  auto out = assemble("LDA $05");
  ASSERT_EQ(out, (std::vector<uint8_t>{0xA5, 0x05}));
}

TEST(AssemblerTest, assemble_zeropage_x_instruction) {
  auto out = assemble("LDA $05,X");
  ASSERT_EQ(out, (std::vector<uint8_t>{0xB5, 0x05}));
}

TEST(AssemblerTest, assemble_zeropage_y_instruction) {
  auto out = assemble("LDX $05,Y");
  ASSERT_EQ(out, (std::vector<uint8_t>{0xB6, 0x05}));
}

TEST(AssemblerTest, assemble_absolute_instruction) {
  auto out = assemble("LDA $0605");
  ASSERT_EQ(out, (std::vector<uint8_t>{0xAD, 0x05, 0x06}));
}

TEST(AssemblerTest, assemble_absolute_x_instruction) {
  auto out = assemble("LDY $0605,X");
  ASSERT_EQ(out, (std::vector<uint8_t>{0xBC, 0x05, 0x06}));
}

TEST(AssemblerTest, assemble_absolute_y_instruction) {
  auto out = assemble("LDA $0605,Y");
  ASSERT_EQ(out, (std::vector<uint8_t>{0xB9, 0x05, 0x06}));
}

TEST(AssemblerTest, assemble_indirect_instruction) {
  auto out = assemble("JMP ($0605)");
  ASSERT_EQ(out, (std::vector<uint8_t>{0x6C, 0x05, 0x06}));
}

TEST(AssemblerTest, assemble_indirect_x_instruction) {
  auto out = assemble("LDA ($05,X)");
  ASSERT_EQ(out, (std::vector<uint8_t>{0xA1, 0x05}));
}

TEST(AssemblerTest, assemble_indirect_y_instruction) {
  auto out = assemble("LDA ($05),Y");
  ASSERT_EQ(out, (std::vector<uint8_t>{0xB1, 0x05}));
}

TEST(AssemblerTest, assemble_multiple_instructions) {
  auto out = assemble("LDA #$05 \n LDA $05");
  ASSERT_EQ(out, (std::vector<uint8_t>{0xA9, 0x05, 0xA5, 0x05}));
}

}  // namespace assembler
}  // namespace nesem
