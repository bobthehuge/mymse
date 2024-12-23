#ifndef COMMON_H
#define COMMON_H

#include <stdarg.h>
#include "../utils/bth_log.h"
#include "../utils/bth_htab.h"

#define ERROR(code, ...) ERR(code, __VA_ARGS__)
#define STRINGIFY(s) #s
#define CSTRNCMP(s1, s2) strncmp((s1), (s2).data, s2.len)

void escprints(char *s);
void nputchar(FILE *file, char c, unsigned int n);
int myasprintf(char **str, const char *fmt, ...);
int myvasprintf(char **str, const char *fmt, va_list args);
void cml_free_vtable(struct htab *htab);

#endif
