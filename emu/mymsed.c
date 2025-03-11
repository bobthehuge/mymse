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
    m68k_flash(&CPU, lines, line_cnt);

    FILE *dd = fopen("dump", "w+");

    for (size_t i = 0; i < M68K_MEM / 32; i++)
    {
        char line[64];
        fdumpf(line, (char *)(CPU.mem + i * 32), 32);
        for (int j = 0; j < 7; j++)
        {
            fwrite(line + j * 8, 1, 8, dd);
            fputc(' ', dd);
        }
        fwrite(line + 56, 1, 8, dd);
        fputc('\n', dd);
    }

    fclose(dd);

    free(lines);
    free(text);
    free(mem);
}
