#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace nesem {
namespace assembler {

std::vector<uint8_t> assemble(std::string input);

}
}  // namespace nesem
