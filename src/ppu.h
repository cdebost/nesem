#pragma once

#include <vector>

#include "cartridge.h"

namespace nesem {

constexpr size_t kDisplayWidth = 256;
constexpr size_t kDisplayHeight = 240;
constexpr uint8_t kTileWidth = 8;
constexpr uint8_t kTileHeight = 8;
constexpr size_t kTilesPerScanline = kDisplayWidth / kTileWidth;

struct PpuAddressLatch {
  uint8_t hi = 0;
  uint8_t lo = 0;

  uint16_t read() const { return (((uint16_t)hi) << 8) | lo; }

  void write(uint8_t val) {
    hi = lo;
    lo = val;
  }

  void increment(uint8_t by) {
    uint16_t val = read() + by;
    hi = val >> 8;
    lo = val & 0xFF;
  }

  void reset() {
    hi = 0;
    lo = 0;
  }
};

struct Ppu {
  std::vector<uint8_t> chr;  // graphics data (external to the PPU)
  ScreenMirroring mirroring = ScreenMirroring::Vertical;
  std::array<uint8_t, 2048> vram = {0};  // video ram (external to the PPU)
  std::array<uint8_t, 32> palettes;      // internal storage for colors
  std::array<uint8_t, 256> oam;          // internal storage for sprites

  // indices into the system color palette for the current frame
  std::array<uint8_t, kDisplayWidth * kDisplayHeight> frame;

  // The current scanline being rendered. 262 scanlines are rendered per frame.
  // Each scanline lasts for 341 PPU clock cycles, each cycle producing one
  // pixel.
  //
  // Scanline # | Purpose
  // -----------|---------------------
  //     -1/261 | Pre-render scanline
  //      0-239 | Visible scanlines
  //        240 | Post-render scanline
  //    241-260 | Vertical blanking
  uint16_t scanline = 0;

  // The current cycle for the current scanline.
  //
  // Cycle # | Purpose
  // --------|---------------------
  //       0 | Idle cycle
  //   1-256 | Fetch data for each tile
  // 257-320 | Fetch data for the _next_ scanline
  // 321-336 | First two tiles for _next_ scanline fetched
  // 337-340 | Unknown
  size_t cycle = 0;

  // Various flags controlling PPU operation.
  //
  // 7  bit  0
  // ---- ----
  // VPHB SINN
  // |||| ||||
  // |||| ||++- Base nametable address
  // |||| ||    (0 = $2000; 1 = $2400; 2 = $2800; 3 = $2C00)
  // |||| |+--- VRAM address increment per CPU read/write of PPUDATA
  // |||| |     (0: add 1, going across; 1: add 32, going down)
  // |||| +---- Sprite pattern table address for 8x8 sprites
  // ||||       (0: $0000; 1: $1000; ignored in 8x16 mode)
  // |||+------ Background pattern table address (0: $0000; 1: $1000)
  // ||+------- Sprite size (0: 8x8 pixels; 1: 8x16 pixels – see PPU OAM#Byte 1)
  // |+-------- PPU master/slave select
  // |          (0: read backdrop from EXT pins; 1: output color on EXT pins)
  // +--------- Generate an NMI at the start of the
  //            vertical blanking interval (0: off; 1: on)
  uint8_t ctrl = 0;

  // Controls the rendering of sprites and backgrounds, as well as colour
  // effects.
  //
  // 7  bit  0
  // ---- ----
  // BGRs bMmG
  // |||| ||||
  // |||| |||+- Greyscale (0: normal color, 1: produce a greyscale display)
  // |||| ||+-- 1: Show background in leftmost 8 pixels of screen, 0: Hide
  // |||| |+--- 1: Show sprites in leftmost 8 pixels of screen, 0: Hide
  // |||| +---- 1: Show background
  // |||+------ 1: Show sprites
  // ||+------- Emphasize red (green on PAL/Dendy)
  // |+-------- Emphasize green (red on PAL/Dendy)
  // +--------- Emphasize blue
  uint8_t mask = 0;

  // Address of OAM memory to access
  uint8_t oam_addr = 0;

  bool nmi_pending = false;

  Ppu() {}
  explicit Ppu(const Cartridge &cartridge);

  // 7  bit  0
  // ---- ----
  // VSO. ....
  // |||| ||||
  // |||+-++++- PPU open bus. Returns stale PPU bus contents.
  // ||+------- Sprite overflow. The intent was for this flag to be set
  // ||         whenever more than eight sprites appear on a scanline, but a
  // ||         hardware bug causes the actual behavior to be more complicated
  // ||         and generate false positives as well as false negatives; see
  // ||         PPU sprite evaluation. This flag is set during sprite
  // ||         evaluation and cleared at dot 1 (the second dot) of the
  // ||         pre-render line.
  // |+-------- Sprite 0 Hit.  Set when a nonzero pixel of sprite 0 overlaps
  // |          a nonzero background pixel; cleared at dot 1 of the pre-render
  // |          line.  Used for raster timing.
  // +--------- Vertical blank has started (0: not in vblank; 1: in vblank).
  //            Set at dot 1 of line 241 (the line *after* the post-render
  //            line); cleared after reading $2002 and at dot 1 of the
  //            pre-render line.
  uint8_t status() const;

  // Read from the specified register. This is the official way that
  // the cpu reads the ppu's registers.
  //
  // The following registers are readable:
  // - Status ($2002)
  // - OAM data ($2004)
  // - Data ($2007)
  // Reading from other registers will return the stale contents of the
  // internal io databus. Otherwise, the io databus is also updated with
  // the bits read.
  uint8_t read(uint16_t addr);

  // Write data to the specified register.
  //
  // The following registers are writable:
  // - Control ($2000)
  // - Mask ($2001)
  // - OAM address ($2003)
  // - OAM data ($2004)
  // - Scroll ($2005)
  // - Address ($2006)
  // - Data ($2007)
  // - OAM DMA ($4014)
  // Writing to status ($2002) is a no-op, but will fill the io databus with
  // the bits, as will writing to any other register.
  void write(uint16_t addr, uint8_t data);

  void tick(size_t cycles);

 private:
  bool in_vblank = false;
  bool sprite_0_hit = false;
  bool sprite_overflow = false;

  // Internal data bus used to communicate w/ the CPU.
  //
  // Writing to any PPU port fills this latch.
  // Reading any readable port also fills this latch with the bits read.
  // Reading from a write-only register returns the latch's current value,
  // as do the unused bits of status.
  uint8_t io_databus = 0;

  // Internal buffer updated only when reading Data register
  uint8_t read_buffer = 0;

  // Used by the scroll and address registers
  PpuAddressLatch addr_latch;

  void draw_scanline();
  uint8_t bg_palette_start_idx(uint16_t tile_row, uint16_t tile_col) const;
};

};  // namespace nesem
