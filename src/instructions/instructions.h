#ifndef MYASM_INSTRUCTIONS
#define MYASM_INSTRUCTIONS

#include "../common/myasm.h"

#define NAME_MAX (32)
#define MNEMONICS_PER_GROUP (20)
#define INSTRUCTION_SIZE (4)

// Static opcodes
#define SOP_LOGICAL_IMM (0b00100100)
#define SOP_LOGICAL_SH_REG (0b00001010)
#define SOP_MOVE_WIDE (0b00100101)
#define SOP_ADD_SUB_IMM (0b00100010)
#define SOP_PC_REl (0b00010000)
#define SOP_EXCEPTIONS (0b11010100)

// ALL VALUES STORED FROM LOWER BIT
typedef struct LogicalImm {
  u8 sf;   // 1 bit sign field = 1 for 64 bit
  u8 opc;  // 2 bit specifies instruction
  u8 s_op; // 6 bit static opcode
  u8 N;    // 1 bit N - Negative flag // doesnt matter ?
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
  LOGICAL_IMM,
  LOGICAL_SH_REG,
  MOVEWIDE,
  ADDSUB_IMM,
  PCRELADDRESSING,
  EXCEPTION,
} InstructionTag;

typedef enum {
  REG_NONE,
  REG_GENERAL,
  REG_ZR,
  REG_SP,
} Register;

typedef struct Instruction {
  const char mnemonic[MNEMONICS_PER_GROUP][NAME_MAX];
  const InstructionTag type;
  union {
    LogicalImm limm;
    LogicalShReg lshreg;
    MoveWide movewide;
    AddSubImm addsub;
    PcRelAddressing pcreladdr;
    Exceptions exception;
  };
} Instruction;

const Instruction *searchInstruction(const char *mnemonic);

#endif

