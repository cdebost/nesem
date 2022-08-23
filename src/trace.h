#pragma once

#include "cpu.h"
#include "nes.h"

namespace nesem {

// Make a string representing the current state of the system.
//
// The string is formatted as follows:
// <pc> <opcode> [<operand1>] [<operand2>] <asm> @ <addr info> <registers>
// <clock cycles> where:
// - <asm> is the assembly representation of the current instruction and its
// operands.
// - <addr info> varies based on the addressing mode of the instruction
std::string trace_explain_state(const Nes &nes);

}  // namespace nesem
