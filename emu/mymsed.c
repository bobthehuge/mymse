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
    readfn(&text, 0, "../samples/sample2.srec");
    size_t line_cnt = 0;
    char **lines = getnlines(text, &line_cnt);

    m68k_trymemflash(&CPU, lines, line_cnt);

    FILE *fd = fopen("dump", "w+");
    m68k_memdump(&CPU, fd);
    fclose(fd);

    free(lines);
    free(text);
    free(mem);
}
