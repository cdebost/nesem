#include "instruction_set.h"

#include <gtest/gtest.h>

namespace nesem {

TEST(InstructionSet, defines_every_possible_opcode) {
  ASSERT_EQ(opcodes.size(), 0x100);
}

}  // namespace nesem
