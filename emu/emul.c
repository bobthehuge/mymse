#include "emul.h"

#define PC(c) (BSWAP32((c)->pc))
#define PC_INC(c,x) ((c)->pc = BSWAP32((PC(c) + (x))))
#define A(c,x) (BSWAP32((c)->areg[x]))
#define D(c,x) (BSWAP32((c)->dreg[x]))

// decode ascii hex `data` and returns decoded raw hex
// results is `n` / 2 bytes long and allocated using calloc
static inline uint8_t *strndecode(const char *data, int n)
{
    uint8_t *m = EMUL_CALLOC(n / 2, 1);
    
    for (int i = 0; i < n; i++)
    {
        if (data[i] >= '0' && data[i] <= '9')
            m[i / 2] |= data[i] - '0';
        else
            m[i / 2] |= data[i] - 'A' + 10;

        if (!(i % 2))
            m[i / 2] <<= 4;
    }

    return m;
}

// encode raw hex `data` (`n` bytes long) to ascii hex in `dst` 
// (need to be at least `n` * 2 bytes long)
static inline void strnencode(char *dst, const char *data, int n)
{
    for (int i = 0; i < n; i++)
    {
        uint8_t h = (data[i] & 0xF0) >> 4;
        uint8_t l = data[i] & 0x0F;

        dst[i*2] = h <= 9 ? (h + '0') : (h - 10 + 'A');
        dst[i*2+1] = l <= 9 ? (l + '0') : (l - 10 + 'A');
    }
}

// encodes and dumps mem to fd (using strnencode)
void m68k_memdump(m68k_cpu *cpu, FILE *fd)
{
    for (size_t i = 0; i < M68K_MEM / 32; i++)
    {
        char line[64];
        strnencode(line, (char *)(cpu->mem + i * 32), 32);
        for (int j = 0; j < 7; j++)
        {
            fwrite(line + j * 8, 1, 8, fd);
            fputc(' ', fd);
        }
        fwrite(line + 56, 1, 8, fd);
        fputc('\n', fd);
    }
}

// decodes and parses SREC char array to raw memory bytes
// see: https://en.wikipedia.org/wiki/SREC_(file_format)
//
Record srec_decode(char *_d)
{
    Record r = {
        .type = -1,
        .count = 0,
        .len = 0,
        .checksum = 0,
        .address = 0,
        .data = NULL
    };

    char *d = _d;

    if (*d++ != 'S')
        return r;

    r.type = (*d++) - '0';

    if (r.type == 4 || r.type > 9)
    {
        r.type = -1;
        return r;
    }

    uint8_t *decoded = strndecode(d, 2);
    r.count = *decoded;
    r.len = *decoded - 1;
    EMUL_FREE(decoded);

    if (r.len < 2)
    {
        r.type = -1;
        return r;
    }

    d += 2;

    decoded = strndecode(d, 8);

    // r.address = BSWAP32(*(uint32_t *)decoded);
    r.address |= decoded[0] << 24;
    r.address |= decoded[1] << 16;
    r.address |= decoded[2] << 8;
    r.address |= decoded[3];

    EMUL_FREE(decoded);

    switch (r.type)
    {
    case 0: case 1: case 5: case 9: // 16 bits address
        r.address >>= 16;
        d += 4;
        r.len -= 2;
        break;
    case 2: case 6: case 8: // 24 bits address
        r.address >>= 8;
        d += 6;
        r.len -= 3;
        break;
    case 3: case 7: // 32 bits address
        d += 8;
        r.len -= 4;
        break;
    default:
        break;
    }

    r.data = strndecode(d, r.len * 2);

    decoded = strndecode(d + r.len * 2, 2);
    r.checksum = *decoded;

    EMUL_FREE(decoded);

    return r;
}

// computes the checksum of `r` and compares it to `r.checksum`
int check_record(Record r)
{
    uint32_t sum = r.count;

    sum += (r.address >> 8) & 0xff;
    sum += (r.address >> 16) & 0xff;
    sum += (r.address >> 24) & 0xff;
    sum += r.address & 0xff;

    for (int i = 0; i < r.len; i++)
        sum += r.data[i];
    
    return r.checksum == (0xFF - (sum & 0xFF));
}

// flashed `n` `lines` (in srec format) to `cpu`'s mem
//
// returns the number of dumped lines until failure
size_t m68k_memflash(m68k_cpu *cpu, char **lines, size_t n)
{
    Record r = {0};
    size_t i = 0;

    for (; i < n; i++)
    {
        r = srec_decode(lines[i]);
        
        if (!check_record(r))
            return i;
        
        EMUL_MEMCPY(cpu->mem + r.address, r.data, r.len);
        EMUL_FREE(r.data);
    }

    return i;
}

// tries to m68k_memflash `n` `lines` to `cpu`'s mem
// on failure, mem is reset to 0
//
// returns if all lines have been dumped
int m68k_trymemflash(m68k_cpu *cpu, char **lines, size_t n)
{
    size_t d = m68k_memflash(cpu, lines, n);

    if (d != n)
    {
        EMUL_LOG("Invalid record count flashed: %zu (expected %zu)\n", d, n);
        EMUL_MEMSET(cpu->mem, 0, M68K_MEM);
    }

    return d == n;
}

// set everything to 0 except memory
void m68k_clear(m68k_cpu *cpu)
{
    uint8_t *mem = cpu->mem;
    EMUL_MEMSET(cpu, 0, sizeof(m68k_cpu));
    cpu->mem = mem;
}

// sets SP and PC from vector table
// see: https://wiki.neogeodev.org/index.php?title=68k_vector_table
// 
void m68k_reset(m68k_cpu *cpu)
{
    cpu->areg[7] = BSWAP32(MEMGET_U32(cpu->mem, 0));
    cpu->pc = BSWAP32(MEMGET_U32(cpu->mem, 1));
}

void m68k_get(uint8_t *addr, uint8_t s, uint32_t *res)
{
    switch (s)
    {
    case 0:
        *res = MEMGET_1B(addr, 0);
        break;
    case 1:
        *res = MEMGET_2B(addr, 0);
        break;
    case 2:
        *res = MEMGET_4B(addr, 0);
        break;
    default:
        EMUL_LOG("%s:%d: Unsupported operation\n", __FILE__, __LINE__);
        break;
    }
}

void m68k_get_pc(m68k_cpu *cpu, uint8_t s, uint32_t *imm)
{
    switch (s)
    {
    case 0:
        *imm = MEMGET_1B(cpu->mem, PC(cpu));
        break;
    case 1:
        *imm = MEMGET_2B(cpu->mem, PC(cpu));
        break;
    case 2:
        *imm = MEMGET_4B(cpu->mem, PC(cpu));
        break;
    default:
        EMUL_LOG("%s:%d: Unsupported operation\n", __FILE__, __LINE__);
        break;
    }
}

void m68k_set(uint8_t *addr, uint8_t s, uint32_t v)
{
    switch (s)
    {
    case 0:
        MEMSET_1B(addr, 0, (uint8_t)v);
        break;
    case 1:
        MEMSET_2B(addr, 0, (uint16_t)v);
        break;
    case 2:
        MEMSET_4B(addr, 0, v);
        break;
    default:
        EMUL_LOG("%s:%d: Unsupported operation\n", __FILE__, __LINE__);
        break;
    }
}

// decodes a Brief Extension Word instruction to an address offset
// bew = 0bMXXXS000DDDDDDDD
void m68k_resolve_brief(m68k_cpu *cpu, uint16_t bew, uint32_t *offset)
{
    uint8_t xn = (bew & 0x7000) >> 12;
    uint8_t size = (bew & 0x0800) >> 11; // >>11 then x4 => >>9
    int8_t disp = (bew & 0xFF);

    uint32_t mask = size == 0 ? 0xFFFF : 0xFFFFFFFF;

    uint32_t off = (bew & 0x8000) ? cpu->areg[xn] : cpu->dreg[xn];
    // off = EMUL_HTOBE32(off);
    off &= mask;

    *offset += size ? (int32_t)off : (int16_t)off + disp;
}

// decodes an smxn instruction to a memory or a register address
// smxn = 0bSSMMMXXX
void m68k_resolve_xn(m68k_cpu *cpu, uint8_t smxn, uint8_t **addr)
{
    uint8_t xn = smxn & 7;
    uint8_t mode = (smxn & 0x38) >> 3;
    uint8_t size = (smxn & 0xC0) >> 5; // >>6 then x2 => >>5
    uint32_t offset = 0;
    
    switch (mode)
    {
    case 0: // DREG
        *addr = (uint8_t *)(cpu->dreg + xn) + (size < 2 ? 3 - size : 0);
        break;
    case 1: // AREG
        *addr = (uint8_t *)(cpu->areg + xn) + (size < 2 ? 3 - size : 0);
        break;
    case 2: // ADDRESS
        *addr = cpu->mem + A(cpu, xn);
        break;
    case 3: // ADDRESS WITH POSTINCREMENT
        // offset = cpu->areg[xn]++ * size;
        *addr = cpu->mem + A(cpu, xn);
        cpu->areg[xn] = BSWAP32(A(cpu, xn) + 1);
        break;
    case 4: // ADDRESS WITH PREDECREMENT
        cpu->areg[xn] = BSWAP32(A(cpu, xn) - 1);
        *addr = cpu->mem + A(cpu, xn);
        break;
    case 5: // ADDRESS WITH DISPLACEMENT
        *addr = (cpu->mem + A(cpu, xn)
            + (int16_t)MEMGET_2B(cpu->mem, PC(cpu)));

        PC_INC(cpu, 2);
        break;
    case 6: // ADDRESS WITH INDEX
        offset += A(cpu, xn);
        m68k_resolve_brief(cpu, MEMGET_2B(cpu->mem, PC(cpu)), &offset);
        PC_INC(cpu, 2);
        *addr = cpu->mem + offset;
        break;
    case 7:
        switch (xn)
        {
        case 0: // ABSOLUTE SHORT
            EMUL_LOG("%s:%d: Unsupported operation\n", __FILE__, __LINE__);
            break;
        case 1: // ABSOLUTE LONG
            EMUL_LOG("%s:%d: Unsupported operation\n", __FILE__, __LINE__);
            break;
        case 2: // PC WITH DISPLACEMENT
            EMUL_LOG("%s:%d: Unsupported operation\n", __FILE__, __LINE__);
            break;
        case 3: // PC WITH INDEX
            EMUL_LOG("%s:%d: Unsupported operation\n", __FILE__, __LINE__);
            break;
        case 4: // IMMEDIATE
            *addr = cpu->mem + PC(cpu);
            PC_INC(cpu, size);
            break;
        default:
            EMUL_LOG("%s:%d: UNREACHABLE (\?)\n", __FILE__, __LINE__);
            break;
        }
        break;
    default:
        EMUL_LOG("%s:%d: UNREACHABLE (\?)\n", __FILE__, __LINE__);
    }
}

void m68k_or(m68k_cpu *cpu, uint8_t up, uint8_t lo)
{
    uint8_t s = (lo & 0xC0) >> 6;
    uint8_t dn = (up & 0xE) >> 1;

    uint32_t target = cpu->dreg[dn];

    uint8_t *dst = 0;
    uint32_t source;

    if (up == 0) // ORI
    {
        m68k_get_pc(cpu, s, &source);
        PC_INC(cpu, 1 << s);
        m68k_resolve_xn(cpu, lo, &dst);
    }
    else
    {
        m68k_resolve_xn(cpu, lo, &dst);
        source = *dst;
        dst = cpu->dreg + dn;

        if (up & 1) // switch direction
        {
            target = source;
            source = *dst;
        }
    }

    uint8_t *dstaddr = (uint8_t *)dst;
    
    if (ISAREGADDR(cpu, dst))
    {
        EMUL_LOG("INV AND of size %d caught at %06X\n",
            1 << s, cpu->pc_cur);
        return;
    }
    else if (ISDREGADDR(cpu, dst))
    {
        dstaddr += s < 2 ? 3 - s : 0;
    }
    else if (!ISMEMADDR(cpu, dstaddr))
    {
        EMUL_LOG("OOB write of size %d caught at %06X\n",
            1 << s, cpu->pc_cur);
        return;
    }

    uint32_t value = target | source;

    m68k_set(dstaddr, s, value);
}

void m68k_and(m68k_cpu *cpu, uint8_t up, uint8_t lo)
{   
    uint8_t s = (lo & 0xC0) >> 6;
    uint8_t dn = (up & 0xE) >> 1;

    uint32_t target = cpu->dreg[dn];

    uint32_t *dst = 0;
    uint32_t source;

    if (up == 2) // ANDI
    {
        m68k_get_pc(cpu, s, &source);
        // cpu->pc = BSWAP32(PC(cpu) + (1 << s));
        PC_INC(cpu, 1 << s);
        m68k_resolve_xn(cpu, lo, &dst);
    }
    else
    {
        m68k_resolve_xn(cpu, lo, &dst);
        source = *dst;
        dst = cpu->dreg + dn;

        if (up & 1) // switch direction
        {
            target = source;
            source = *dst;
        }
    }

    uint8_t *dstaddr = (uint8_t *)dst;
    
    if (ISDREGADDR(cpu, dst))
    {
        dstaddr += s < 2 ? 3 - s : 0;
    }
    else if (ISAREGADDR(cpu, dst))
    {
        EMUL_LOG("INV AND of size %d caught at %06X\n",
            1 << s, cpu->pc_cur);
        return;
    }
    else if (!ISMEMADDR(cpu, dstaddr))
    {
        EMUL_LOG("OOB write of size %d caught at %06X\n",
            1 << s, cpu->pc_cur);
        return;
    }

    uint32_t value = target & source;

    m68k_set(dstaddr, s, BSWAP32(value));
}

void m68k_move(m68k_cpu *cpu, uint8_t up, uint8_t lo)
{
    uint8_t s = (4 - (up >> 4)) % 3;
    uint8_t dst_smxn = (s << 6) |
        (((up >> 1) & 7) | ((lo & 0xC0) >> 3) | (up & 1) << 5);
    uint8_t src_smxn = (s << 6) | lo;

    uint32_t *src;
    m68k_resolve_xn(cpu, src_smxn, &src);
    
    uint32_t *dst;
    m68k_resolve_xn(cpu, dst_smxn, &dst);

    uint8_t *srcaddr = (uint8_t *)src;
    uint8_t *dstaddr = (uint8_t *)dst;

    if (ISAREGADDR(cpu, dst) || ISDREGADDR(cpu, dst))
    {
        if (ISAREGADDR(cpu, dst) && s == 0)
        {
            EMUL_LOG("INV write of size %d caught at %06X\n",
                1 << s, cpu->pc_cur);
            return;
        }
        dstaddr += s < 2 ? 3 - s : 0;
    }
    else if (!ISMEMADDR(cpu, dstaddr))
    {
        EMUL_LOG("OOB write of size %d caught at %06X\n",
            1 << s, cpu->pc_cur);
        return;
    }

    if (ISAREGADDR(cpu, src) || ISDREGADDR(cpu, src))
    {
        srcaddr += s < 2 ? 3 - s : 0;
    }
    else if (!ISMEMADDR(cpu, srcaddr))
    {
        EMUL_LOG("OOB read of size %d caught at %06X\n",
            1 << s, cpu->pc_cur);
        return;
    }

    uint32_t srcval;
    m68k_get(srcaddr, s, &srcval);
    m68k_set(dstaddr, s, srcval);
}

// executes 1 instruction fetched from MEMORY[PC]
// see: http://goldencrystal.free.fr/M68kOpcodes-v2.3.pdf
// 
void m68k_cycle(m68k_cpu *cpu)
{
    cpu->pc_cur = PC(cpu);
    
    uint8_t up = MEMGET_U8(cpu->mem, PC(cpu));
    PC_INC(cpu, 1);
    uint8_t lo = MEMGET_U8(cpu->mem, PC(cpu));
    PC_INC(cpu, 1);

    switch (up >> 4)
    {
    case 0: // ORI to CC - MOVEP
        {
            // BTST, BCHG, BCLR, BSET
            if ((up & 0x0F) % 2 == 1)
            {
            }
            else
            {
                switch (up & 0x0E)
                {
                case 0: // ORI
                    EMUL_LOG("ORI at '%06X'\n", cpu->pc_cur);
                    m68k_or(cpu, up, lo);
                    break;
                case 2: // ANDI
                    EMUL_LOG("ANDI at '%06X'\n", cpu->pc_cur);
                    m68k_and(cpu, up, lo);
                    break;
                case 4: // SUBI
                    break;
                case 6: // ADDI
                    break;
                case 8: // BTST, BCHG, BCLR, BSET, MOVEP (immediate)
                    break;
                case 10: // EORI
                    break;
                case 12: // CMPI
                    break;
                }
            }
        }
        break;
    case 1: case 2: case 3: // MOVEA, MOVE
        {
            EMUL_LOG("MOVE at '%06X'\n", cpu->pc_cur);
            m68k_move(cpu, up, lo);
        }
        break;
    case 4: // MOVE from SR - CHK
        {
            if (up == 0x4A && lo == 0xFC)
                EMUL_LOG("ILLEGAL at '%06X'\n", cpu->pc_cur);
        }
        break;
    case 5: // ADDQ, SUBQ, Scc, DBcc
        {
        }
        break;
    case 6: // BRA, BSR, Bcc
        {
        }
        break;
    case 7: // MOVEQ
        {
        }
        break;
    case 8: // DIVU, DIVS, SBCD, OR 
        {
            if ((up & 1) && (lo >> 3) <= 2)
            {
                if ((lo >> 6) == 3) // DIVU / DIVS
                    EMUL_LOG("DIVU/S at '%06X'\n", cpu->pc_cur);
                if ((lo >> 3) <= 2) // SBCD
                    EMUL_LOG("SBCD at '%06X'\n", cpu->pc_cur);
            }
            else // OR
            {
                EMUL_LOG("OR at '%06X'\n", cpu->pc_cur);
                m68k_or(cpu, up, lo);
            }
        }
        break;
    case 9: // SUB, SUBX, SUBA
        {
        }
        break;
    case 11: // EOR, CMPM, CMP, CMPA
        {
        }
        break;
    case 12: // MULU, MULS, ABCD, EXG, AND
        {
            EMUL_LOG("AND at '%06X'\n", cpu->pc_cur);
            m68k_and(cpu, up, lo);
        }
        break;
    case 13: // ADDI, ADDX, ADDA
        {
        }
        break;
    case 14: // ASd, LSd, ROXd
        {
        }
        break;
    default:
        EMUL_LOG("unknown instruction %04X at pc %06X\n",
            (((uint16_t)up << 8) | lo), cpu->pc_cur);
        break;
    }
}
