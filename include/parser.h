#ifndef MYASM_PARSER
#define MYASM_PARSER

#include "myasm.h"

typedef enum RegisterType {
  R0,   R1,   R2,   R3,   R4,   R5,   R6,
  R7,   R8,   R9,   R10,  R11,  R12,  R13, 
  R14,  R15,  R16,  R17,  R18,  R19,  R20, 
  R21,  R22,  R23,  R24,  R25,  R26,  R27,  
  R28,  R29,  RLR,  RZR,  RSP,
} RegisterType;

typedef struct {
  bool extended;
  RegisterType reg;
} Register;

typedef enum {
  UXTB,  UXTH,  UXTW,  UXTX,
  SXTB,  SXTH,  SXTW,  SXTX,
} ExtendType;

struct ExtendMap {
  const char* value;
  const ExtendType type;
};

static const struct ExtendMap EXTENDS[] = {
  {"uxtb", UXTB},
  {"uxth", UXTH},
  {"uxtw", UXTW},
  {"uxtx", UXTX},
  {"sxtb", SXTB},
  {"sxth", SXTH},
  {"sxtw", SXTW},
  {"sxtx", SXTX},
};

typedef enum {
  SH_LSL, SH_LSR, SH_ASR, SH_ROR,
} ShiftType;

struct ShiftMap {
  const char* value;
  const ShiftType type;
};

static const struct ShiftMap SHIFTS[] = {
  {"lsl", SH_LSL}, 
  {"lsr", SH_LSR}, 
  {"asr", SH_ASR}, 
  {"ror", SH_ROR},
};

typedef enum {
  EQ = 0,    NE = 1,    CS = 2,    CC = 3,
  MI = 4,    PL = 5,    VS = 6,    VC = 7,
  HI = 8,    LS = 9,    GE = 10,   LT = 11,
  GT = 12,   LE = 13,   AL = 14,   NV = 15,
} ConditionType;

struct ConditionMap {
  const char* value;
  const ConditionType type;
};

static const struct ConditionMap CONDITIONS[] = {
    {"eq", EQ},
    {"ne", NE},
    {"cs", CS},
    {"cc", CC},
    {"mi", MI},
    {"pl", PL},
    {"vs", VS},
    {"vc", VC},
    {"hi", HI},
    {"ls", LS},
    {"ge", GE},
    {"lt", LT},
    {"gt", GT},
    {"le", LE},
    {"al", AL},
    {"nv", NV},
};

u8 parseLabelDeclaration(char *label);
u8 parseRegister(const char *reg, Register *r);
u8 parseImmediateU8(const char *imm, u8 *res);
u8 parseImmediateU16(const char *imm, u16 *res);
u8 parseImmediateU32(const char *imm, u32 *res);
u8 parseImmediateU64(const char *imm, u64 *res);
u8 parseImmediateI64(const char *imm, i64 *res);
u8 parseExtend(const char *extend, ExtendType *ex);
u8 parseShift(const char *shift, ShiftType *sh);
u8 parseCondition(const char *condition, ConditionType *c);
u8 isString(const char *str);

#endif
