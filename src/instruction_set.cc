#include "instruction_set.h"

namespace nesem {

const std::vector<Opcode> opcodes = {
    {0x00, "BRK", AddressingMode::Implied, 1, 7},
    {0x01, "ORA", AddressingMode::IndirectX, 2, 6},
    {0x02, "JAM", AddressingMode::Immediate, 1, 0, kIllegalOpcode},
    {0x03, "SLO", AddressingMode::IndirectX, 2, 8, kIllegalOpcode},
    {0x04, "NOP", AddressingMode::Zeropage, 2, 3, kIllegalOpcode},
    {0x05, "ORA", AddressingMode::Zeropage, 2, 3},
    {0x06, "ASL", AddressingMode::Zeropage, 2, 5},
    {0x07, "SLO", AddressingMode::Zeropage, 2, 5, kIllegalOpcode},
    {0x08, "PHP", AddressingMode::Implied, 1, 3},
    {0x09, "ORA", AddressingMode::Immediate, 2, 2},
    {0x0A, "ASL", AddressingMode::Implied, 1, 2},
    {0x0B, "ANC", AddressingMode::Immediate, 2, 2, kIllegalOpcode},
    {0x0C, "NOP", AddressingMode::Absolute, 3, 4, kIllegalOpcode},
    {0x0D, "ORA", AddressingMode::Absolute, 3, 4},
    {0x0E, "ASL", AddressingMode::Absolute, 3, 6},
    {0x0F, "SLO", AddressingMode::Absolute, 3, 6, kIllegalOpcode},
    {0x10, "BPL", AddressingMode::Relative, 2, 2},
    {0x11, "ORA", AddressingMode::IndirectY, 2, 5,
     kAddCycleIfPageBoundaryCrossed},
    {0x12, "JAM", AddressingMode::Immediate, 1, 0, kIllegalOpcode},
    {0x13, "SLO", AddressingMode::IndirectY, 2, 8, kIllegalOpcode},
    {0x14, "NOP", AddressingMode::ZeropageX, 2, 4, kIllegalOpcode},
    {0x15, "ORA", AddressingMode::ZeropageX, 2, 4},
    {0x16, "ASL", AddressingMode::ZeropageX, 2, 6},
    {0x17, "SLO", AddressingMode::ZeropageX, 2, 6, kIllegalOpcode},
    {0x18, "CLC", AddressingMode::Implied, 1, 2},
    {0x19, "ORA", AddressingMode::AbsoluteY, 3, 4,
     kAddCycleIfPageBoundaryCrossed},
    {0x1A, "NOP", AddressingMode::Implied, 1, 2, kIllegalOpcode},
    {0x1B, "SLO", AddressingMode::AbsoluteY, 3, 7, kIllegalOpcode},
    {0x1C, "NOP", AddressingMode::AbsoluteX, 3, 4,
     kIllegalOpcode | kAddCycleIfPageBoundaryCrossed},
    {0x1D, "ORA", AddressingMode::AbsoluteX, 3, 4,
     kAddCycleIfPageBoundaryCrossed},
    {0x1E, "ASL", AddressingMode::AbsoluteX, 3, 7},
    {0x1F, "SLO", AddressingMode::AbsoluteX, 3, 7, kIllegalOpcode},
    {0x20, "JSR", AddressingMode::Absolute, 3, 6},
    {0x21, "AND", AddressingMode::IndirectX, 2, 6},
    {0x22, "JAM", AddressingMode::Immediate, 1, 0, kIllegalOpcode},
    {0x23, "RLA", AddressingMode::IndirectX, 2, 8, kIllegalOpcode},
    {0x24, "BIT", AddressingMode::Zeropage, 2, 3},
    {0x25, "AND", AddressingMode::Zeropage, 2, 3},
    {0x26, "ROL", AddressingMode::Zeropage, 2, 5},
    {0x27, "RLA", AddressingMode::Zeropage, 2, 5, kIllegalOpcode},
    {0x28, "PLP", AddressingMode::Implied, 1, 4},
    {0x29, "AND", AddressingMode::Immediate, 2, 2},
    {0x2A, "ROL", AddressingMode::Implied, 1, 2},
    {0x2B, "ANC", AddressingMode::Immediate, 2, 2, kIllegalOpcode},
    {0x2C, "BIT", AddressingMode::Absolute, 3, 4},
    {0x2D, "AND", AddressingMode::Absolute, 3, 4},
    {0x2E, "ROL", AddressingMode::Absolute, 3, 6},
    {0x2F, "RLA", AddressingMode::Absolute, 3, 6, kIllegalOpcode},
    {0x30, "BMI", AddressingMode::Relative, 2, 2},
    {0x31, "AND", AddressingMode::IndirectY, 2, 5,
     kAddCycleIfPageBoundaryCrossed},
    {0x32, "JAM", AddressingMode::Immediate, 1, 0, kIllegalOpcode},
    {0x33, "RLA", AddressingMode::IndirectY, 2, 8, kIllegalOpcode},
    {0x34, "NOP", AddressingMode::ZeropageX, 2, 4, kIllegalOpcode},
    {0x35, "AND", AddressingMode::ZeropageX, 2, 4},
    {0x36, "ROL", AddressingMode::ZeropageX, 2, 6},
    {0x37, "RLA", AddressingMode::ZeropageX, 2, 6, kIllegalOpcode},
    {0x38, "SEC", AddressingMode::Implied, 1, 2},
    {0x39, "AND", AddressingMode::AbsoluteY, 3, 4,
     kAddCycleIfPageBoundaryCrossed},
    {0x3A, "NOP", AddressingMode::Implied, 1, 2, kIllegalOpcode},
    {0x3B, "RLA", AddressingMode::AbsoluteY, 3, 7, kIllegalOpcode},
    {0x3C, "NOP", AddressingMode::AbsoluteX, 3, 4,
     kIllegalOpcode | kAddCycleIfPageBoundaryCrossed},
    {0x3D, "AND", AddressingMode::AbsoluteX, 3, 4,
     kAddCycleIfPageBoundaryCrossed},
    {0x3E, "ROL", AddressingMode::AbsoluteX, 3, 7},
    {0x3F, "RLA", AddressingMode::AbsoluteX, 3, 7, kIllegalOpcode},
    {0x40, "RTI", AddressingMode::Implied, 1, 6},
    {0x41, "EOR", AddressingMode::IndirectX, 2, 6},
    {0x42, "JAM", AddressingMode::Immediate, 1, 0, kIllegalOpcode},
    {0x43, "SRE", AddressingMode::IndirectX, 2, 8, kIllegalOpcode},
    {0x44, "NOP", AddressingMode::Zeropage, 2, 3, kIllegalOpcode},
    {0x45, "EOR", AddressingMode::Zeropage, 2, 3},
    {0x46, "LSR", AddressingMode::Zeropage, 2, 5},
    {0x47, "SRE", AddressingMode::Zeropage, 2, 5, kIllegalOpcode},
    {0x48, "PHA", AddressingMode::Implied, 1, 3},
    {0x49, "EOR", AddressingMode::Immediate, 2, 2},
    {0x4A, "LSR", AddressingMode::Implied, 1, 2},
    {0x4B, "ALR", AddressingMode::Immediate, 2, 2, kIllegalOpcode},
    {0x4C, "JMP", AddressingMode::Absolute, 3, 3},
    {0x4D, "EOR", AddressingMode::Absolute, 3, 4},
    {0x4E, "LSR", AddressingMode::Absolute, 3, 6},
    {0x4F, "SRE", AddressingMode::Absolute, 3, 6, kIllegalOpcode},
    {0x50, "BVC", AddressingMode::Relative, 2, 2},
    {0x51, "EOR", AddressingMode::IndirectY, 2, 5,
     kAddCycleIfPageBoundaryCrossed},
    {0x52, "JAM", AddressingMode::Immediate, 1, 0, kIllegalOpcode},
    {0x53, "SRE", AddressingMode::IndirectY, 2, 8, kIllegalOpcode},
    {0x54, "NOP", AddressingMode::ZeropageX, 2, 4, kIllegalOpcode},
    {0x55, "EOR", AddressingMode::ZeropageX, 2, 4},
    {0x56, "LSR", AddressingMode::ZeropageX, 2, 6},
    {0x57, "SRE", AddressingMode::ZeropageX, 2, 6, kIllegalOpcode},
    {0x58, "CLI", AddressingMode::Implied, 1, 2},
    {0x59, "EOR", AddressingMode::AbsoluteY, 3, 4,
     kAddCycleIfPageBoundaryCrossed},
    {0x5A, "NOP", AddressingMode::Implied, 1, 2, kIllegalOpcode},
    {0x5B, "SRE", AddressingMode::AbsoluteY, 3, 7, kIllegalOpcode},
    {0x5C, "NOP", AddressingMode::AbsoluteX, 3, 4,
     kIllegalOpcode | kAddCycleIfPageBoundaryCrossed},
    {0x5D, "EOR", AddressingMode::AbsoluteX, 3, 4,
     kAddCycleIfPageBoundaryCrossed},
    {0x5E, "LSR", AddressingMode::AbsoluteX, 3, 7},
    {0x5F, "SRE", AddressingMode::AbsoluteX, 3, 7, kIllegalOpcode},
    {0x60, "RTS", AddressingMode::Implied, 1, 6},
    {0x61, "ADC", AddressingMode::IndirectX, 2, 6},
    {0x62, "JAM", AddressingMode::Immediate, 1, 0, kIllegalOpcode},
    {0x63, "RRA", AddressingMode::IndirectX, 2, 8, kIllegalOpcode},
    {0x64, "NOP", AddressingMode::Zeropage, 2, 3, kIllegalOpcode},
    {0x65, "ADC", AddressingMode::Zeropage, 2, 3},
    {0x66, "ROR", AddressingMode::Zeropage, 2, 5},
    {0x67, "RRA", AddressingMode::Zeropage, 2, 5, kIllegalOpcode},
    {0x68, "PLA", AddressingMode::Implied, 1, 4},
    {0x69, "ADC", AddressingMode::Immediate, 2, 2},
    {0x6A, "ROR", AddressingMode::Implied, 1, 2},
    {0x6B, "ARR", AddressingMode::Immediate, 2, 2, kIllegalOpcode},
    {0x6C, "JMP", AddressingMode::Indirect, 3, 5},
    {0x6D, "ADC", AddressingMode::Absolute, 3, 4},
    {0x6E, "ROR", AddressingMode::Absolute, 3, 6},
    {0x6F, "RRA", AddressingMode::Absolute, 3, 6, kIllegalOpcode},
    {0x70, "BVS", AddressingMode::Relative, 2, 2},
    {0x71, "ADC", AddressingMode::IndirectY, 2, 5,
     kAddCycleIfPageBoundaryCrossed},
    {0x72, "JAM", AddressingMode::Immediate, 1, 0, kIllegalOpcode},
    {0x73, "RRA", AddressingMode::IndirectY, 2, 8, kIllegalOpcode},
    {0x74, "NOP", AddressingMode::ZeropageX, 2, 4, kIllegalOpcode},
    {0x75, "ADC", AddressingMode::ZeropageX, 2, 4},
    {0x76, "ROR", AddressingMode::ZeropageX, 2, 6},
    {0x77, "RRA", AddressingMode::ZeropageX, 2, 6, kIllegalOpcode},
    {0x78, "SEI", AddressingMode::Implied, 1, 2},
    {0x79, "ADC", AddressingMode::AbsoluteY, 3, 4,
     kAddCycleIfPageBoundaryCrossed},
    {0x7A, "NOP", AddressingMode::Implied, 1, 2, kIllegalOpcode},
    {0x7B, "RRA", AddressingMode::AbsoluteY, 3, 7, kIllegalOpcode},
    {0x7C, "NOP", AddressingMode::AbsoluteX, 3, 4,
     kIllegalOpcode | kAddCycleIfPageBoundaryCrossed},
    {0x7D, "ADC", AddressingMode::AbsoluteX, 3, 4,
     kAddCycleIfPageBoundaryCrossed},
    {0x7E, "ROR", AddressingMode::AbsoluteX, 3, 7},
    {0x7F, "RRA", AddressingMode::AbsoluteX, 3, 7, kIllegalOpcode},
    {0x80, "NOP", AddressingMode::Immediate, 2, 2, kIllegalOpcode},
    {0x81, "STA", AddressingMode::IndirectX, 2, 6},
    {0x82, "NOP", AddressingMode::Immediate, 2, 2, kIllegalOpcode},
    {0x83, "SAX", AddressingMode::IndirectX, 2, 6, kIllegalOpcode},
    {0x84, "STY", AddressingMode::Zeropage, 2, 3},
    {0x85, "STA", AddressingMode::Zeropage, 2, 3},
    {0x86, "STX", AddressingMode::Zeropage, 2, 3},
    {0x87, "SAX", AddressingMode::Zeropage, 2, 3, kIllegalOpcode},
    {0x88, "DEY", AddressingMode::Implied, 1, 2},
    {0x89, "NOP", AddressingMode::Immediate, 2, 2, kIllegalOpcode},
    {0x8A, "TXA", AddressingMode::Implied, 1, 2},
    {0x8B, "ANE", AddressingMode::Immediate, 2, 2,
     kIllegalOpcode | kHighlyUnstableOpcode},
    {0x8C, "STY", AddressingMode::Absolute, 3, 4},
    {0x8D, "STA", AddressingMode::Absolute, 3, 4},
    {0x8E, "STX", AddressingMode::Absolute, 3, 4},
    {0x8F, "SAX", AddressingMode::Absolute, 3, 4, kIllegalOpcode},
    {0x90, "BCC", AddressingMode::Relative, 2, 2},
    {0x91, "STA", AddressingMode::IndirectY, 2, 6},
    {0x92, "JAM", AddressingMode::Immediate, 1, 0, kIllegalOpcode},
    {0x93, "SHA", AddressingMode::IndirectY, 2, 6,
     kIllegalOpcode | kUnstableOpcode},
    {0x94, "STY", AddressingMode::ZeropageX, 2, 4},
    {0x95, "STA", AddressingMode::ZeropageX, 2, 4},
    {0x96, "STX", AddressingMode::ZeropageY, 2, 4},
    {0x97, "SAX", AddressingMode::ZeropageY, 2, 4, kIllegalOpcode},
    {0x98, "TYA", AddressingMode::Implied, 1, 2},
    {0x99, "STA", AddressingMode::AbsoluteY, 3, 5},
    {0x9A, "TXS", AddressingMode::Implied, 1, 2},
    {0x9B, "TAS", AddressingMode::AbsoluteY, 3, 5,
     kIllegalOpcode | kUnstableOpcode},
    {0x9C, "SHY", AddressingMode::AbsoluteX, 3, 5,
     kIllegalOpcode | kUnstableOpcode},
    {0x9D, "STA", AddressingMode::AbsoluteX, 3, 5},
    {0x9E, "SHX", AddressingMode::AbsoluteY, 3, 5,
     kIllegalOpcode | kUnstableOpcode},
    {0x9F, "SHA", AddressingMode::AbsoluteY, 3, 5,
     kIllegalOpcode | kUnstableOpcode},
    {0xA0, "LDY", AddressingMode::Immediate, 2, 2},
    {0xA1, "LDA", AddressingMode::IndirectX, 2, 6},
    {0xA2, "LDX", AddressingMode::Immediate, 2, 2},
    {0xA3, "LAX", AddressingMode::IndirectX, 2, 6, kIllegalOpcode},
    {0xA4, "LDY", AddressingMode::Zeropage, 2, 3},
    {0xA5, "LDA", AddressingMode::Zeropage, 2, 3},
    {0xA6, "LDX", AddressingMode::Zeropage, 2, 3},
    {0xA7, "LAX", AddressingMode::Zeropage, 2, 3, kIllegalOpcode},
    {0xA8, "TAY", AddressingMode::Implied, 1, 2},
    {0xA9, "LDA", AddressingMode::Immediate, 2, 2},
    {0xAA, "TAX", AddressingMode::Implied, 1, 2},
    {0xAB, "LXA", AddressingMode::Immediate, 1, 2,
     kIllegalOpcode | kHighlyUnstableOpcode},
    {0xAC, "LDY", AddressingMode::Absolute, 3, 4},
    {0xAD, "LDA", AddressingMode::Absolute, 3, 4},
    {0xAE, "LDX", AddressingMode::Absolute, 3, 4},
    {0xAF, "LAX", AddressingMode::Absolute, 3, 4, kIllegalOpcode},
    {0xB0, "BCS", AddressingMode::Relative, 2, 2},
    {0xB1, "LDA", AddressingMode::IndirectY, 2, 5,
     kAddCycleIfPageBoundaryCrossed},
    {0xB2, "SHA", AddressingMode::Immediate, 1, 0,
     kIllegalOpcode | kUnstableOpcode},
    {0xB3, "LAX", AddressingMode::IndirectY, 2, 5,
     kIllegalOpcode | kAddCycleIfPageBoundaryCrossed},
    {0xB4, "LDY", AddressingMode::ZeropageX, 2, 4},
    {0xB5, "LDA", AddressingMode::ZeropageX, 2, 4},
    {0xB6, "LDX", AddressingMode::ZeropageY, 2, 4},
    {0xB7, "LAX", AddressingMode::ZeropageY, 2, 4, kIllegalOpcode},
    {0xB8, "CLV", AddressingMode::Implied, 1, 2},
    {0xB9, "LDA", AddressingMode::AbsoluteY, 3, 4,
     kAddCycleIfPageBoundaryCrossed},
    {0xBA, "TSX", AddressingMode::Implied, 1, 2},
    {0xBB, "LAS", AddressingMode::AbsoluteY, 3, 4, kIllegalOpcode},
    {0xBC, "LDY", AddressingMode::AbsoluteX, 3, 4,
     kAddCycleIfPageBoundaryCrossed},
    {0xBD, "LDA", AddressingMode::AbsoluteX, 3, 4,
     kAddCycleIfPageBoundaryCrossed},
    {0xBE, "LDX", AddressingMode::AbsoluteY, 3, 4,
     kAddCycleIfPageBoundaryCrossed},
    {0xBF, "LAX", AddressingMode::AbsoluteY, 3, 4,
     kIllegalOpcode | kAddCycleIfPageBoundaryCrossed},
    {0xC0, "CPY", AddressingMode::Immediate, 2, 2},
    {0xC1, "CMP", AddressingMode::IndirectX, 2, 6},
    {0xC2, "NOP", AddressingMode::Immediate, 2, 2, kIllegalOpcode},
    {0xC3, "DCP", AddressingMode::IndirectX, 2, 8, kIllegalOpcode},
    {0xC4, "CPY", AddressingMode::Zeropage, 2, 3},
    {0xC5, "CMP", AddressingMode::Zeropage, 2, 3},
    {0xC6, "DEC", AddressingMode::Zeropage, 2, 5},
    {0xC7, "DCP", AddressingMode::Zeropage, 2, 5, kIllegalOpcode},
    {0xC8, "INY", AddressingMode::Implied, 1, 2},
    {0xC9, "CMP", AddressingMode::Immediate, 2, 2},
    {0xCA, "DEX", AddressingMode::Implied, 1, 2},
    {0xCB, "SBX", AddressingMode::Immediate, 2, 2, kIllegalOpcode},
    {0xCC, "CPY", AddressingMode::Absolute, 3, 4},
    {0xCD, "CMP", AddressingMode::Absolute, 3, 4},
    {0xCE, "DEC", AddressingMode::Absolute, 3, 6},
    {0xCF, "DCP", AddressingMode::Absolute, 3, 6, kIllegalOpcode},
    {0xD0, "BNE", AddressingMode::Relative, 2, 2},
    {0xD1, "CMP", AddressingMode::IndirectY, 2, 5,
     kAddCycleIfPageBoundaryCrossed},
    {0xD2, "JAM", AddressingMode::Immediate, 1, 0, kIllegalOpcode},
    {0xD3, "DCP", AddressingMode::IndirectY, 2, 8, kIllegalOpcode},
    {0xD4, "NOP", AddressingMode::ZeropageX, 2, 4, kIllegalOpcode},
    {0xD5, "CMP", AddressingMode::ZeropageX, 2, 4},
    {0xD6, "DEC", AddressingMode::ZeropageX, 2, 6},
    {0xD7, "DCP", AddressingMode::ZeropageX, 2, 6, kIllegalOpcode},
    {0xD8, "CLD", AddressingMode::Implied, 1, 2},
    {0xD9, "CMP", AddressingMode::AbsoluteY, 3, 4,
     kAddCycleIfPageBoundaryCrossed},
    {0xDA, "NOP", AddressingMode::Implied, 1, 2, kIllegalOpcode},
    {0xDB, "DCP", AddressingMode::AbsoluteY, 3, 7, kIllegalOpcode},
    {0xDC, "NOP", AddressingMode::AbsoluteX, 3, 4,
     kIllegalOpcode | kAddCycleIfPageBoundaryCrossed},
    {0xDD, "CMP", AddressingMode::AbsoluteX, 3, 4,
     kAddCycleIfPageBoundaryCrossed},
    {0xDE, "DEC", AddressingMode::AbsoluteX, 3, 7},
    {0xDF, "DCP", AddressingMode::AbsoluteX, 3, 7, kIllegalOpcode},
    {0xE0, "CPX", AddressingMode::Immediate, 2, 2},
    {0xE1, "SBC", AddressingMode::IndirectX, 2, 6},
    {0xE2, "NOP", AddressingMode::Immediate, 2, 2, kIllegalOpcode},
    {0xE3, "ISB", AddressingMode::IndirectX, 2, 8, kIllegalOpcode},
    {0xE4, "CPX", AddressingMode::Zeropage, 2, 3},
    {0xE5, "SBC", AddressingMode::Zeropage, 2, 3},
    {0xE6, "INC", AddressingMode::Zeropage, 2, 5},
    {0xE7, "ISB", AddressingMode::Zeropage, 2, 5, kIllegalOpcode},
    {0xE8, "INX", AddressingMode::Implied, 1, 2},
    {0xE9, "SBC", AddressingMode::Immediate, 2, 2},
    {0xEA, "NOP", AddressingMode::Implied, 1, 2},
    {0xEB, "SBC", AddressingMode::Immediate, 2, 2, kIllegalOpcode},
    {0xEC, "CPX", AddressingMode::Absolute, 3, 4},
    {0xED, "SBC", AddressingMode::Absolute, 3, 4},
    {0xEE, "INC", AddressingMode::Absolute, 3, 6},
    {0xEF, "ISB", AddressingMode::Absolute, 3, 6, kIllegalOpcode},
    {0xF0, "BEQ", AddressingMode::Relative, 2, 2},
    {0xF1, "SBC", AddressingMode::IndirectY, 2, 5,
     kAddCycleIfPageBoundaryCrossed},
    {0xF2, "JAM", AddressingMode::Immediate, 1, 0, kIllegalOpcode},
    {0xF3, "ISB", AddressingMode::IndirectY, 2, 8, kIllegalOpcode},
    {0xF4, "NOP", AddressingMode::ZeropageX, 2, 4, kIllegalOpcode},
    {0xF5, "SBC", AddressingMode::ZeropageX, 2, 4},
    {0xF6, "INC", AddressingMode::ZeropageX, 2, 6},
    {0xF7, "ISB", AddressingMode::ZeropageX, 2, 6, kIllegalOpcode},
    {0xF8, "SED", AddressingMode::Implied, 1, 2},
    {0xF9, "SBC", AddressingMode::AbsoluteY, 3, 4,
     kAddCycleIfPageBoundaryCrossed},
    {0xFA, "NOP", AddressingMode::Implied, 1, 2, kIllegalOpcode},
    {0xFB, "ISB", AddressingMode::AbsoluteY, 3, 7, kIllegalOpcode},
    {0xFC, "NOP", AddressingMode::AbsoluteX, 3, 4,
     kIllegalOpcode | kAddCycleIfPageBoundaryCrossed},
    {0xFD, "SBC", AddressingMode::AbsoluteX, 3, 4,
     kAddCycleIfPageBoundaryCrossed},
    {0xFE, "INC", AddressingMode::AbsoluteX, 3, 7},
    {0xFF, "ISB", AddressingMode::AbsoluteX, 3, 7, kIllegalOpcode},
};

}  // namespace nesem
