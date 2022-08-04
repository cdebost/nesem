#include "cartridge.h"

#include <fmt/core.h>

#include <array>
#include <cstdlib>

namespace nesem {

constexpr size_t PRG_ROM_PAGE_SIZE = 16 * 1024;
constexpr size_t CHR_ROM_PAGE_SIZE = 8 * 1024;

// magic string "NES^Z"
constexpr std::array<char, 4> NES_TAG = {0x4E, 0x45, 0x53, 0x1A};

Cartridge load_ines_rom_dump(std::istream *is) {
  Cartridge cart;

  std::array<char, 16> header;
  is->read(&header[0], 16);
  if (is->gcount() != 16)
    throw std::runtime_error(
        fmt::format("Failed to read header from cartridge: only {} bytes read",
                    is->gcount()));

  if (memcmp(header.data(), NES_TAG.data(), 4 * sizeof(uint8_t)) != 0)
    throw std::runtime_error("Cartridge does not have expected NES tag");

  cart.mapper = (header[7] & 0b11110000) | (header[6] >> 4);
  if (cart.mapper != 0)
    throw std::runtime_error(
        fmt::format("Unsupported cartridge mapper {}", cart.mapper));

  uint8_t ines_ver = (header[7] >> 2) & 0b00000011;
  if (ines_ver != 0)
    throw std::runtime_error(
        fmt::format("Unsupport ines version {}", ines_ver));

  if ((header[6] & 0b1000) != 0) {
    cart.mirroring = ScreenMirroring::FourScreen;
  } else {
    int vertmirror = (header[6] & 0b1) != 0;
    cart.mirroring =
        vertmirror ? ScreenMirroring::Vertical : ScreenMirroring::Horizontal;
  }

  size_t prgl = (size_t)header[4] * PRG_ROM_PAGE_SIZE;
  size_t chrl = (size_t)header[5] * CHR_ROM_PAGE_SIZE;

  int trainer = header[6] & 0b100;
  int prg_rom_start = 16 + (trainer ? 512 : 0);
  is->seekg(prg_rom_start);

  cart.prg.insert(cart.prg.begin(), prgl, 0);
  is->read((char *)&cart.prg[0], prgl);
  if (is->gcount() != prgl)
    throw std::runtime_error(fmt::format(
        "Failed to read prg from cartridge: only {} bytes read", is->gcount()));

  cart.chr.insert(cart.chr.begin(), chrl, 0);
  is->read((char *)&cart.chr[0], chrl);
  if (is->gcount() != chrl)
    throw std::runtime_error(fmt::format(
        "Failed to read chr from cartridge: only {} bytes read", is->gcount()));

  return cart;
}

void Cartridge::write_prg(size_t size, const std::vector<uint8_t> &prg) {
  std::copy(prg.begin(), prg.end(), std::back_inserter(this->prg));
  this->prg.resize(size, 0);
  this->prg[0xFFFC - 0x8000] = 0x00;
  this->prg[0xFFFC - 0x8000 + 1] = 0x80;
}

}  // namespace nesem
