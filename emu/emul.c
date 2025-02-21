#include <stdlib.h>
#include <string.h>

#include "emul.h"

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
void fdumpf(char *dst, const char *data, int n)
{
    for (int i = 0; i < n; i++)
    {
        uint8_t h = (data[i] & 0xF0) >> 4;
        uint8_t l = data[i] & 0x0F;

        dst[i*2] = h <= 9 ? (h + '0') : (h - 10 + 'A');
        dst[i*2+1] = l <= 9 ? (l + '0') : (l - 10 + 'A');
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
    free(decoded);

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

    free(decoded);

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

    free(decoded);

    return r;
}

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

size_t m68k_dump2mem(m68k_cpu *cpu, char **lines, size_t n)
{
    Record r = {0};
    size_t i = 0;

    for (; i < n; i++)
    {
        r = srec_decode(lines[i]);
        
        if (!check_record(r))
            return i;
        
        memcpy(cpu->mem + r.address, r.data, r.len);
    }

    return i;
}

int m68k_flash(m68k_cpu *cpu, char **lines, size_t n)
{
    size_t d = m68k_dump2mem(cpu, lines, n);

    if (d != n)
    {
        EMUL_LOG("Invalid record count dumped: %zu (expected %zu)\n", d, n);
        memset(cpu->mem, 0, M68K_MEM);
    }

    return d == n;
}

inline void m68k_cycle(m68k_cpu *cpu)
{
}
