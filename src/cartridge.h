// Implementation of ROM dumps using the iNES format:
// https://www.nesdev.org/wiki/INES

// ROM dumps contain 4 sections:
//  Header      16B
//  Trainer     512B    (optional)
//  PRG ROM     size varies
//  CHR ROM     size varies
//
// The header is as follows:
// 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
//  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
//  |  |  |  |  |  |  |  |  |  |  |  +--+--+--+--+- reserved
//  |  |  |  |  |  |  |  |  |  +--+---------------- unused
//  |  |  |  |  |  |  |  |  +---------------------- number of 8KB PRG RAM units
//  |  |  |  |  |  |  |  +------------------------- control byte 2
//  |  |  |  |  |  |  +---------------------------- control byte 1
//  |  |  |  |  |  +------------------------------- number of 8KB VROM banks
//  (CHR ROM) |  |  |  |  +---------------------------------- number of 16KB ROM
//  banks (PRG ROM)
//  +--+--+--+------------------------------------- String "NES^Z"
//
// Control byte 1 is as follows:
// 7654 3210
// |||| ||||
// |||| |||+- 1 for vertical mirroring, 0 for horizontal
// |||| ||+-- 1 for battery-backed RAM at $6000-$7FFF
// |||| |+--- 1 for a 512-byte trainer at $7000-$71FF
// |||| +---- 1 for a four-screen VRAM layout
// ||||
// ++++------ lower bits of ROM mapper type
//
// Control byte 2 is as follows:
// 7654 3210
// |||| ||||
// |||| |||+- 0 for iNES 1.0
// |||| ||+-- 0 for iNES 1.0
// |||| ++--- 10 for iNES 2.0, 00 for iNES 1.0
// ||||
// ++++------ upper bits of ROM mapper type

#pragma once

#include <array>
#include <cstdint>
#include <iostream>
#include <vector>

namespace nesem {

enum class ScreenMirroring { Vertical, Horizontal, FourScreen };

struct Cartridge {
  std::vector<uint8_t> prg;  // Bytes containing game code
  std::vector<uint8_t> chr;  // Bytes containing graphics data
  uint8_t mapper =
      0;  // Type of mapper. Some mappers provide access to more ROM.
  ScreenMirroring mirroring;  // Type of screen mirroring for the PPU

  // Write data into PRG ROM.
  // param size: should be 0x4000 (for a mirrored cartridge) or 0x8000
  // param prg: the program data itself
  void write_prg(size_t size, const std::vector<uint8_t> &prg);
};

Cartridge load_ines_rom_dump(std::istream *is);

}  // namespace nesem
