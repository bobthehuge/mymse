// MIT License
// 
// Copyright (c) 2024 bobthehuge
// 
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to 
// deal in the Software without restriction, including without limitation the 
// rights to use, copy, modify, merge, publish, distribute, sublicense, and/or 
// sell copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included in 
// all copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING 
// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER 
// DEALINGS IN THE SOFTWARE.

#ifndef BTH_LEX_H
#define BTH_LEX_H

#include <stdlib.h>

enum BTH_LEX_KIND
{
    INVALID,
    LK_END,
    LK_IDENT,
    LK_SYMBOL,
    LK_DELIMITED,
    BTH_LEX_KIND_COUNT
};

struct bth_lex_token
{
    size_t kind;
    size_t idx; // index if SYMBOL or DELIM else 0
    const char *name;
    const char *filename;
    size_t row;
    size_t col;
    const char *begin;
    const char *end;
};

struct bth_lexer
{
    const char *buffer;
    const char *filename;
    size_t size;

    size_t cur;
    size_t col;
    size_t row;

    const char **symbols;
    size_t symbols_count;
    const char **delims;
    size_t delims_count;

    void *usrdata;
};

#ifndef BTH_LEX_STRNCMP
#  define BTH_LEX_STRNCMP(s1, s2, n) (strncmp(s1, s2, (n)))
#endif

#ifndef BTH_LEX_STRLEN
#  include <string.h>
#  define BTH_LEX_STRLEN(s) (strlen(s))
#endif

#ifndef BTH_LEX_GET_DELIM
#  ifndef BTH_LEX_ERRX
#    include <err.h>
#    define BTH_LEX_ERRX(code, fmt, ...) errx((code), fmt, __VA_ARGS__)
#  endif

#  define BTH_LEX_DEFAULT_GET_DELIM
#  define BTH_LEX_GET_DELIM(l, t) bth_lex_get_delim(l, t)
#endif

#ifndef BTH_LEX_GET_SYMBOL
#  define BTH_LEX_DEFAULT_GET_SYMBOL
#  define BTH_LEX_GET_SYMBOL(l, t) bth_lex_get_symbol(l, t)
#endif

#ifndef BTH_LEX_GET_IDENT
#  ifndef BTH_LEX_ISVALID
#    include <ctype.h>
#    define BTH_LEX_DEFAULT_ISVALID
#    define BTH_LEX_ISVALID(c) bth_lex_isvalid((c))
#  endif

#  define BTH_LEX_DEFAULT_GET_IDENT
#  define BTH_LEX_GET_IDENT(l, t) bth_lex_get_ident(l, t)
#endif

const char *bth_lex_kind2str(size_t id);
struct bth_lex_token bth_lex_get_token(struct bth_lexer *lex);

#endif /* ! */
