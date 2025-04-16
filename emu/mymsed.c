// #include <raylib.h>
#include <stdlib.h>

#define BTH_IO_IMPLEMENTATION
#include "../utils/bth_io.h"

#define BTH_STRING_IMPLEMENTATION
#include "../utils/bth_string.h"

#include "emul.h"

int main(void)
{
    uint8_t *mem = calloc(M68K_MEM, 1);
    m68k_cpu CPU = {.mem = mem};

    char *text = NULL;
    size_t len = readfn(&text, 0, "../samples/sample3.srec");

    text[len - 1] = 0; // get rid of the double newline at EOF
    
    size_t line_cnt = 0;
    char **lines = getnlines(text, &line_cnt);
    
    m68k_trymemflash(&CPU, lines, line_cnt);

    m68k_reset(&CPU);

    m68k_cycle(&CPU); // move.l ..,d3
    m68k_cycle(&CPU); // move.w d4,a0
    m68k_cycle(&CPU); // move.l ..,d4
    // m68k_cycle(&CPU); // btst.l d4,d3
    // m68k_cycle(&CPU); // btst.l ..,d3
    // CPU.pc = BSWAP32(BSWAP32(CPU.pc) + 2);
    m68k_cycle(&CPU); // ori
    m68k_cycle(&CPU); // ori

    m68k_cycle(&CPU); // move.l ..,d2
    m68k_cycle(&CPU); // move.l ..,d1
    m68k_cycle(&CPU); // or.l d1,d2

    m68k_cycle(&CPU); // andi
    m68k_cycle(&CPU); // andi

    m68k_cycle(&CPU); // move.l ..,d4
    m68k_cycle(&CPU); // move.l ..,d1
    m68k_cycle(&CPU); // and.l d4,d1

    m68k_cycle(&CPU); // illegal

    printf("\n");
    printf("d0: %08X  d1: %08X  d2: %08X  d3: %08X\n",
        BSWAP32(CPU.dreg[0]),
        BSWAP32(CPU.dreg[1]),
        BSWAP32(CPU.dreg[2]),
        BSWAP32(CPU.dreg[3])
    );
    printf("d4: %08X  d5: %08X  d6: %08X  d7: %08X\n",
        BSWAP32(CPU.dreg[4]),
        BSWAP32(CPU.dreg[5]),
        BSWAP32(CPU.dreg[6]),
        BSWAP32(CPU.dreg[7])
    );

    printf("\n");
    printf("a0: %08X  a1: %08X  a2: %08X  a3: %08X\n",
        BSWAP32(CPU.areg[0]),
        BSWAP32(CPU.areg[1]),
        BSWAP32(CPU.areg[2]),
        BSWAP32(CPU.areg[3])
    );
    printf("a4: %08X  a5: %08X  a6: %08X  a7: %08X\n",
        BSWAP32(CPU.areg[4]),
        BSWAP32(CPU.areg[5]),
        BSWAP32(CPU.areg[6]),
        BSWAP32(CPU.areg[7])
    );

    printf("\n");
    printf("PC: %08X\n", BSWAP32(CPU.pc));
    
    FILE *fd = fopen("dump", "w+");
    m68k_memdump(&CPU, fd);
    fclose(fd);

    free(lines);
    free(text);
    free(mem);
}
