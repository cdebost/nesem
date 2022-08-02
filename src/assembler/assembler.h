#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace nesem {
namespace assembler {

std::vector<uint8_t> assemble(const std::string &input);

}
}  // namespace nesem
