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
    CPU.dreg[3] = 0xCAFE0ABE;
    CPU.pc += 4; // cafebabe
    m68k_cycle(&CPU); // move.l ..,d4
    CPU.dreg[4] = 4;
    CPU.pc += 4; // 00000004
    m68k_cycle(&CPU); // btst.l d4,d3
    m68k_cycle(&CPU); // btst.l ..,d3
    CPU.pc += 2; // 0004
    m68k_cycle(&CPU); // ori
    m68k_cycle(&CPU); // ori
    m68k_cycle(&CPU); // illegal

    FILE *fd = fopen("dump", "w+");
    m68k_memdump(&CPU, fd);
    fclose(fd);

    printf("%08X\n", MEMGET_2B(CPU.mem, 0xac4));

    free(lines);
    free(text);
    free(mem);
}
