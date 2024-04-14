#ifndef MYASM_ARM_INSTRUCTIONS
#define MYASM_ARM_INSTRUCTIONS

#include "lexer.h"

#define ARM_INSTRUCTION_SIZE (4)

typedef struct LogicalImm {
  u8 sf;   // 1 bit sign field = 1 for 64 bit
  u8 opc;  // 2 bit specifies instruction
  u8 s_op; // 6 bit static opcode
  u8 N;    // 1 bit immediate
  u8 immr; // 6 bit immediate
  u8 imms; // 6 bit immediate
  u8 Rn;   // 5 bit register
  u8 Rd;   // 5 bit register
} LogicalImm;

typedef struct LogicalShReg {
  u8 sf;   // 1 bit sign field = 1 for 64 bit
  u8 opc;  // 2 bit specifies instruction
  u8 s_op; // 5 bit static opcode
  u8 sh;   // 2 bit shift
  u8 N;    // 1 bit N - Negative flag
  u8 Rm;   // 5 bit register
  u8 imm6; // 6 bit immediate
  u8 Rn;   // 5 bit register
  u8 Rd;   // 5 bit register
} LogicalShReg;

typedef struct MoveWide {
  u8 sf;     // 1 bit sign field = 1 for 64 bit
  u8 opc;    // 2 bit specifies instruction
  u8 s_op;   // 6 bit static opcode
  u8 hw;     // 2 bit hw - ?
  u16 imm16; // 16 bit immediate
  u8 Rd;     // 5 bit register
} MoveWide;

typedef struct AddSubImm {
  u8 sf;     // 1 bit sign field = 1 for 64 bit
  u8 op;     // 1 bit 0 = ADD, 1 = SUB
  u8 S;      // 1 bit 1 = setting flags
  u8 s_op;   // 6 bit static opcode
  u8 sh;     // 1 bit shift
  u16 imm12; // 12 bit immediate
  u8 Rn;     // 5 bit register
  u8 Rd;     // 5 bit register
} AddSubImm;

typedef struct PcRelAddressing {
  u8 op;     // 1 bit specifies instruction
  u8 immlo;  // 2 bit immediate low
  u8 sop;    // 5 bit static opcode
  u32 immhi; // 19 bit immediate high
  u8 Rd;     // 5 bit register
} PcRelAddressing;

typedef struct Excpetions {
  u8 sop;    // 8 bit of static opcode
  u8 opc;    // 3 bit specifies instruction
  u16 imm16; // 16 bit immediate
  u8 op2;    // 3 bit that always == 000
  u8 LL;     // 2 bit specifies instruction
} Exceptions;

typedef enum {
  NO_ARG = 0,
  IMMEDIATE = 1,
  REGISTER = 2,
  SP = 4,
  LABEL = 8,
  SHIFT = 16,
  EXTEND = 32,
  OPTIONAL = 128,
} Argument;

typedef struct Signature {
  u8 n_args;
  u8 a1;
  u8 a2;
  u8 a3;
  u8 a4;
  u8 a5;
  u8 a6;
  u8 a7;
} Signature;

typedef enum {
  NONE,
  LOGICAL_IMM,
  LOGICAL_SH_REG,
  MOVEWIDE,
  ADDSUB_IMM,
  PCRELADDRESSING,
  EXCEPTION,
} InstructionType;


typedef struct Instruction {
  char const *const mnemonic[10];
  const InstructionType type;
  const Signature s;
} Instruction;

#endif
