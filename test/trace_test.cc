#include "trace.h"

#include <gtest/gtest.h>

#include "assembler/assembler.h"
#include "mmu.h"

namespace nesem {

class TraceTest : public ::testing::Test {
protected:
  RamOnlyMmu mmu;
  Cpu cpu{&mmu};
};

TEST_F(TraceTest, registers) {
  cpu.pc = 0x1234;
  cpu.a = 1;
  cpu.x = 2;
  cpu.y = 3;
  cpu.flags.interrupt_disable = false;
  cpu.flags.carry = true;
  cpu.sp = 0xFA;
  cpu.cycles = 654321;

  ASSERT_EQ(
      "1234  00        BRK                             A:01 X:02 Y:03 P:21 "
      "SP:FA CYC:654321",
      trace_explain_state(cpu));
}

TEST_F(TraceTest, illegal_instruction) {
  cpu.pc = 0;
  mmu.write(0x0000, 0x1A);

  ASSERT_EQ(
      "0000  1A       *NOP                             A:00 X:00 Y:00 P:24 "
      "SP:FD CYC:0",
      trace_explain_state(cpu));
}

TEST_F(TraceTest, assembly) {
  cpu.pc = 0;
  cpu.x = 1;
  cpu.y = 1;

  // Target address for operand
  mmu.write(0x10, 0x22);
  // Second byte of target address for operand (for absolute instructions)
  mmu.write(0x11, 0x01);
  // Memory referenced to by operand
  mmu.write(0x0122, 0x33);
  mmu.write(0x0123, 0x34);

  mmu.write(0x0000, 0xA5);
  mmu.write(0x0001, 0x10);
  ASSERT_EQ(
      "0000  A5 10     LDA $10 = 22                    A:00 X:01 Y:01 P:24 "
      "SP:FD CYC:0",
      trace_explain_state(cpu));
  
  mmu.write(0x0000, 0xB5);
  mmu.write(0x0001, 0x0F);
  ASSERT_EQ(
      "0000  B5 0F     LDA $0F,X @ 10 = 22             A:00 X:01 Y:01 P:24 "
      "SP:FD CYC:0",
      trace_explain_state(cpu));
  
  mmu.write(0x0000, 0xAD);
  mmu.write(0x0001, 0x10);
  mmu.write(0x0002, 0x00);
  ASSERT_EQ(
      "0000  AD 10 00  LDA $0010 = 22                  A:00 X:01 Y:01 P:24 "
      "SP:FD CYC:0",
      trace_explain_state(cpu));

  mmu.write(0x0000, 0xBD);
  mmu.write(0x0001, 0x0F);
  mmu.write(0x0002, 0x00);
  ASSERT_EQ(
      "0000  BD 0F 00  LDA $000F,X @ 0010 = 22         A:00 X:01 Y:01 P:24 "
      "SP:FD CYC:0",
      trace_explain_state(cpu));
  
  mmu.write(0x0000, 0xA1);
  mmu.write(0x0001, 0x0F);
  ASSERT_EQ(
      "0000  A1 0F     LDA ($0F,X) @ 10 = 0122 = 33    A:00 X:01 Y:01 P:24 "
      "SP:FD CYC:0",
      trace_explain_state(cpu));

  mmu.write(0x0000, 0xB1);
  mmu.write(0x0001, 0x10);
  ASSERT_EQ(
      "0000  B1 10     LDA ($10),Y = 0122 @ 0123 = 34  A:00 X:01 Y:01 P:24 "
      "SP:FD CYC:0",
      trace_explain_state(cpu));

  mmu.write(0x0000, 0xF0);
  mmu.write(0x0001, 0x10);
  ASSERT_EQ(
      "0000  F0 10     BEQ $0012                       A:00 X:01 Y:01 P:24 "
      "SP:FD CYC:0",
      trace_explain_state(cpu));
}

}  // namespace nesem
