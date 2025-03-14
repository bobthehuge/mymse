#include "emul.h"

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

    // r.address = EMUL_HTOBE32(*(uint32_t *)decoded);
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
    cpu->areg[7] = M68K_GET_U32(cpu, 0);
    cpu->pc = M68K_GET_U32(cpu, 1);
}

// executes 1 instruction fetched from MEMORY[PC]
// see: http://goldencrystal.free.fr/M68kOpcodes-v2.3.pdf
// 
void m68k_cycle(m68k_cpu *cpu)
{
    uint8_t up = M68K_GET_U8(cpu, cpu->pc);
    uint8_t lo = M68K_GET_U8(cpu, cpu->pc);

    // ORI to CC - MOVEP
    if (up >> 4 == 0)
    {
        // BTST, BCHG, BCLR, BSET, MOVEP
        if ((up & 0x0F) % 2 == 1)
        {
        }
        else
        {
            switch (up & 0x0E)
            {
            case 0: // ORI
                break;
            case 2: // ANDI
                break;
            case 4: // SUBI
                break;
            case 6: // ADDI
                break;
            case 8: // BTST, BCHG, BCLR, BSET, MOVEP
                break;
            case 10: // EORI
                break;
            case 12: // CMPI
                break;
            }
        }
    }
}
