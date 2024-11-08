#ifndef CML_TOKEN_H
#define CML_TOKEN_H

#include <stdint.h>
#include <stdio.h>

#include "../utils/bth_cstr.h"

#define CML_TKIND_KW_OFFSET TK_ILLEGAL
#define CML_KW_COUNT TK_IDENT - TK_ILLEGAL

#define TOKEN_EMPTY {.row = 0, .col = 0, .kind = END, .value = NULL}

enum cml_tkind
{
    END = 0, // should ALWAYS be the first kind
    TK_INVALID,
    // TK_INT8,
    // TK_INT16,
    // TK_INT32,
    TK_SEMI,
    TK_RSLASH,
    TK_COMMA,
    TK_POUND,
    TK_REG_A,
    TK_REG_D,
    TK_ORI, // 1st keyword
    TK_ANDI,
    TK_SUBI,
    TK_ADDI,
    TK_EORI,
    TK_CMPI,
    TK_BTST,
    TK_BCHG,
    TK_BCLR,
    TK_BSET,
    TK_MOVEP,
    TK_MOVEA,
    TK_MOVE,
    TK_NEGX,
    TK_CLR,
    TK_NEG,
    TK_NOT,
    TK_EXT,
    TK_NBCD,
    TK_SWAP,
    TK_PEA,
    TK_ILLEGAL,
    TK_TAS,
    TK_TST,
    TK_TRAP,
    TK_LINK,
    TK_UNLK,
    TK_RESET,
    TK_NOP,
    TK_STOP,
    TK_RTE,
    TK_RTS,
    TK_TRAPV,
    TK_RTR,
    TK_JSR,
    TK_JMP,
    TK_MOVEM,
    TK_LEA,
    TK_CHK,
    TK_ADDQ,
    TK_SUBQ,
    TK_SCC,
    TK_DBCC,
    TK_BRA,
    TK_BSR,
    TK_BCC,
    TK_MOVEQ,
    TK_DIVU,
    TK_DIVS,
    TK_SBCD,
    TK_OR,
    TK_SUB,
    TK_SUBX,
    TK_SUBA,
    TK_EOR,
    TK_CMPM,
    TK_CMP,
    TK_CMPA,
    TK_MULU,
    TK_MULS,
    TK_ABCD,
    TK_EXG,
    TK_AND,
    TK_ADD,
    TK_ADDX,
    TK_ADDA,
    TK_ASD,
    TK_LSD,
    TK_ROXD,
    TK_ROD,
    TK_DC,
    TK_ORG,
    TK_EQU,
    TK_VEC,
    TK_IDENT, // should ALWAYS be AFTER the last keyword kind
    TK_LABEL, // alphanum ident
};

struct cml_token
{
    uint32_t row;
    uint32_t col;
    enum cml_tkind kind;
    char *value;
};

const char *cml_tkind2str(enum cml_tkind kind);

#endif
