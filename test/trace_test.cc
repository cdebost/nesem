#include "trace.h"

#include <gtest/gtest.h>

#include "assembler/assembler.h"
#include "mmu.h"

namespace nesem {

class TraceTest : public ::testing::Test {
 protected:
  Nes nes;
};

TEST_F(TraceTest, registers) {
  nes.cpu.pc = 0x1234;
  nes.cpu.a = 1;
  nes.cpu.x = 2;
  nes.cpu.y = 3;
  nes.cpu.flags.interrupt_disable = false;
  nes.cpu.flags.carry = true;
  nes.cpu.sp = 0xFA;
  nes.cpu.cycles = 654321;
  nes.mmu.ppu.scanline = 50;
  nes.mmu.ppu.cycle = 7;

  ASSERT_EQ(
      "1234  00        BRK                             A:01 X:02 Y:03 P:21 "
      "SP:FA PPU: 50,  7 CYC:654321",
      trace_explain_state(nes));
}

TEST_F(TraceTest, illegal_instruction) {
  nes.cpu.pc = 0;
  nes.mmu.write(0x0000, 0x1A);

  ASSERT_EQ(
      "0000  1A       *NOP                             A:00 X:00 Y:00 P:24 "
      "SP:FD PPU:  0,  0 CYC:0",
      trace_explain_state(nes));
}

TEST_F(TraceTest, assembly) {
  nes.cpu.pc = 0;
  nes.cpu.x = 1;
  nes.cpu.y = 1;

  // Target address for operand
  nes.mmu.write(0x10, 0x22);
  // Second byte of target address for operand (for absolute instructions)
  nes.mmu.write(0x11, 0x01);
  // Memory referenced to by operand
  nes.mmu.write(0x0122, 0x33);
  nes.mmu.write(0x0123, 0x34);

  nes.mmu.write(0x0000, 0xA5);
  nes.mmu.write(0x0001, 0x10);
  ASSERT_EQ(
      "0000  A5 10     LDA $10 = 22                    A:00 X:01 Y:01 P:24 "
      "SP:FD PPU:  0,  0 CYC:0",
      trace_explain_state(nes));

  nes.mmu.write(0x0000, 0xB5);
  nes.mmu.write(0x0001, 0x0F);
  ASSERT_EQ(
      "0000  B5 0F     LDA $0F,X @ 10 = 22             A:00 X:01 Y:01 P:24 "
      "SP:FD PPU:  0,  0 CYC:0",
      trace_explain_state(nes));

  nes.mmu.write(0x0000, 0xAD);
  nes.mmu.write(0x0001, 0x10);
  nes.mmu.write(0x0002, 0x00);
  ASSERT_EQ(
      "0000  AD 10 00  LDA $0010 = 22                  A:00 X:01 Y:01 P:24 "
      "SP:FD PPU:  0,  0 CYC:0",
      trace_explain_state(nes));

  nes.mmu.write(0x0000, 0xBD);
  nes.mmu.write(0x0001, 0x0F);
  nes.mmu.write(0x0002, 0x00);
  ASSERT_EQ(
      "0000  BD 0F 00  LDA $000F,X @ 0010 = 22         A:00 X:01 Y:01 P:24 "
      "SP:FD PPU:  0,  0 CYC:0",
      trace_explain_state(nes));

  nes.mmu.write(0x0000, 0xA1);
  nes.mmu.write(0x0001, 0x0F);
  ASSERT_EQ(
      "0000  A1 0F     LDA ($0F,X) @ 10 = 0122 = 33    A:00 X:01 Y:01 P:24 "
      "SP:FD PPU:  0,  0 CYC:0",
      trace_explain_state(nes));

  nes.mmu.write(0x0000, 0xB1);
  nes.mmu.write(0x0001, 0x10);
  ASSERT_EQ(
      "0000  B1 10     LDA ($10),Y = 0122 @ 0123 = 34  A:00 X:01 Y:01 P:24 "
      "SP:FD PPU:  0,  0 CYC:0",
      trace_explain_state(nes));

  nes.mmu.write(0x0000, 0xF0);
  nes.mmu.write(0x0001, 0x10);
  ASSERT_EQ(
      "0000  F0 10     BEQ $0012                       A:00 X:01 Y:01 P:24 "
      "SP:FD PPU:  0,  0 CYC:0",
      trace_explain_state(nes));
}

}  // namespace nesem
