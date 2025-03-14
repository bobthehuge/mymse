#pragma once
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#ifndef M68K_MEM
#define M68K_MEM 0x01000000
#endif

#define SSR_C_MASK 0x0001
#define SSR_V_MASK 0x0002
#define SSR_Z_MASK 0x0004
#define SSR_N_MASK 0x0008
#define SSR_X_MASK 0x0010
#define SSR_I_MASK 0x00E0
#define SSR_S_MASK 0x0100
#define SSR_T_MASK 0x0200

#define SSR_CARRY(i)         (i & SSR_C_MASK)
#define SSR_OVERFLOW(i)      ((i & SSR_V_MASK) >> 1)
#define SSR_ZERO(i)          ((i & SSR_Z_MASK) >> 2)
#define SSR_NEGATIVE(i)      ((i & SSR_N_MASK) >> 3)
#define SSR_EXTEND(i)        ((i & SSR_X_MASK) >> 4)
#define SSR_INTERRUPT(i)     ((i & SSR_I_MASK) >> 5)
#define SSR_SUPERVISOR(i)    ((i & SSR_S_MASK) >> 8)
#define SSR_TRACE(i)         ((i & SSR_T_MASK) >> 9)

#ifndef EMUL_HTOBE32
#    ifdef BIG_ENDIAN
#        define EMUL_HTOBE32(x) (x)
#    else
#        include <byteswap.h>
#        define EMUL_HTOBE32(x) bswap_32(x)
#    endif
#endif

#ifndef EMUL_MEMCPY
#define EMUL_MEMCPY(s, d, l) memcpy(s, d, l)
#endif

#ifndef EMUL_MEMSET
#define EMUL_MEMSET(d, v, l) memset(d, v, l)
#endif

#ifndef EMUL_CALLOC
#define EMUL_CALLOC(x, n) calloc(x, n)
#endif

#ifndef EMUL_FREE
#define EMUL_FREE(x) free(x)
#endif

#ifndef EMUL_LOG
#include <stdio.h>
#define EMUL_LOG(fmt, ...) printf(fmt, __VA_ARGS__)
#endif

typedef struct {
    uint32_t dreg[8];
    uint32_t areg[8];
    uint32_t pc;
    uint32_t ssp;
    uint32_t usp;
    uint16_t ssr;
    uint8_t *mem;
} m68k_cpu;

#define M68K_GET_U32(c, n) (*(uint32_t *)((c)->mem + (n) * 8))
#define M68K_GET_U16(c, n) (*(uint16_t *)((c)->mem + (n) * 4))
#define M68K_GET_U8(c, n) ((c)->mem[n])

// Records with type outside of [0-9] are errors
// S4 records are also errors
// checksums are not verified there, they're simply decoded
// len is the data length in bytes (when omitting checksum and address)
typedef struct {
    uint8_t  type;
    uint8_t  count;
    uint8_t  len;
    uint8_t  checksum;
    uint32_t address;
    uint8_t *data;
} Record;

// encodes and dumps mem to fd (using strnencode)
void m68k_memdump(m68k_cpu *cpu, FILE *fd);

// computes the checksum of `r` and compares it to `r.checksum`
int check_record(Record r);

// flashed `n` `lines` (in srec format) to `cpu`'s mem
//
// returns the number of dumped lines until failure
size_t m68k_memflash(m68k_cpu *cpu, char **lines, size_t n);

// tries to m68k_memflash `n` `lines` to `cpu`'s mem
// on failure, mem is reset to 0
//
// returns if all lines have been dumped
int m68k_trymemflash(m68k_cpu *cpu, char **lines, size_t n);

// set everything to 0 except memory
void m68k_clear(m68k_cpu *cpu);

// sets SP and PC from vector table
// https://wiki.neogeodev.org/index.php?title=68k_vector_table
// 
void m68k_reset(m68k_cpu *cpu);

// executes 1 instruction fetched from MEMORY[PC]
// see: http://goldencrystal.free.fr/M68kOpcodes-v2.3.pdf
// 
void m68k_cycle(m68k_cpu *cpu);
