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

#include "bth_lex.h"

#define TRUE 1
#define FALSE 0

static const char *__kind_names[BTH_LEX_KIND_COUNT] = {
    [INVALID]         = "INVALID",
    [LK_END]          = "END",
    [LK_IDENT]        = "IDENT",
    [LK_SYMBOL]       = "SYMBOL",
    [LK_DELIMITED]    = "DELIMITED",
};

const char *bth_lex_kind2str(size_t id)
{
    return __kind_names[id];
}

#ifdef BTH_LEX_DEFAULT_GET_DELIM
int bth_lex_find_delim(struct bth_lexer *lex, const char *s2, size_t *idx)
{
    for (size_t i = 0; i < lex->delims_count; i++)
    {
        const char *s1 = lex->delims[i * 3 + 1];
        if (!BTH_LEX_STRNCMP(s1, s2, BTH_LEX_STRLEN(s1)))
        {
            *idx = i * 3;
            return TRUE;
        }
    }

    return FALSE;
}

int bth_lex_get_delim(struct bth_lexer *lex, struct bth_lex_token *t)
{
    const char *curptr = lex->buffer + lex->cur;
    size_t idx = 0;

    if (!bth_lex_find_delim(lex, curptr, &idx))
        return FALSE;
    
    const char **delim = lex->delims + idx;

    // size_t clen = 0;
    size_t clen = BTH_LEX_STRLEN(delim[1]);
    size_t lend = BTH_LEX_STRLEN(delim[2]);

    t->kind = LK_DELIMITED;
    t->idx = idx;
    t->begin = curptr;
    t->row = lex->row;
    t->col = lex->col;

    while (BTH_LEX_STRNCMP(delim[2], curptr + clen, lend))
    {
        if (lex->cur + clen >= lex->size - lend)
            BTH_LEX_ERRX(1, "Unclosed delimiter %s at l:%zu c:%zu", 
                    delim[0], lex->row, lex->col);

        if (*(curptr + clen) == '\n')
        {
            lex->row++;
            lex->col = 0;
        }

        clen++;
    }

    t->name = delim[0];
    t->end = curptr + clen + lend;
    lex->col += clen + lend;
    lex->cur += clen + lend;

    if (*(curptr + clen) == '\n')
    {
        lex->row++;
        lex->col = 0;
    }

    return TRUE;
}
#endif

#ifdef BTH_LEX_DEFAULT_GET_SYMBOL
int bth_lex_find_symbol(struct bth_lexer *lex, const char *s2, size_t *idx)
{
    for (size_t i = 0; i < lex->symbols_count; i++)
    {
        const char *s1 = lex->symbols[i * 2 + 1];
        if (!BTH_LEX_STRNCMP(s1, s2, BTH_LEX_STRLEN(s1)))
        {
            *idx = i * 2;
            return TRUE;
        }
    }

    return FALSE;
}

int bth_lex_get_symbol(struct bth_lexer *lex, struct bth_lex_token *t)
{
    const char *curptr = lex->buffer + lex->cur;
    size_t idx = 0;

    if (!bth_lex_find_symbol(lex, curptr, &idx))
        return FALSE;
    
    const char **symbol = lex->symbols + idx;
    size_t slen = BTH_LEX_STRLEN(symbol[1]);

    t->kind = LK_SYMBOL;
    t->idx = idx;
    t->name = symbol[0];
    t->row = lex->row;
    t->col = lex->col;
    t->begin = curptr;
    t->end = curptr + slen;

    lex->col += slen;

    if (!BTH_LEX_STRNCMP("\n", symbol[1], slen))
    {
        lex->row++;
        lex->col = 0;
    }

    lex->cur += slen;
    
    return TRUE;
}
#endif


#ifdef BTH_LEX_DEFAULT_GET_IDENT

#ifdef BTH_LEX_DEFAULT_ISVALID
int bth_lex_isvalid(char c)
{
    return isalnum(c) || c == '_';
}
#endif

int bth_lex_get_ident(struct bth_lexer *lex, struct bth_lex_token *t)
{
    const char *curptr = lex->buffer + lex->cur;
    const char *lastptr = lex->buffer + lex->size;
    size_t off = 0;

    while (curptr + off < lastptr && BTH_LEX_ISVALID(*(curptr + off)))
        off++;

    if (off == 0)
        return FALSE;
    
    t->kind = LK_IDENT;
    t->idx = 0;
    t->name = "IDENT";
    t->row = lex->row;
    t->col = lex->col;
    t->begin = curptr;
    t->end = curptr + off;

    lex->cur += off;
    lex->col += off;

    return TRUE;
}
#endif

// is in bounds ?
// is delim ?
// is symbol ?
// is valid ident ?

struct bth_lex_token bth_lex_get_token(struct bth_lexer *lex)
{
    struct bth_lex_token tok = {0};

    if (lex->cur >= lex->size)
    {
        tok.kind = LK_END;
        tok.idx = 0;
        tok.name = "END";
        tok.row = lex->row;
        tok.col = lex->col;
        tok.begin = lex->buffer + lex->size;
        tok.end = tok.begin;
        return tok;
    }

    if (BTH_LEX_GET_DELIM(lex, &tok))
        return tok;

    if (BTH_LEX_GET_SYMBOL(lex, &tok))
        return tok;

    if (BTH_LEX_GET_IDENT(lex, &tok))
        return tok;

    return tok;
}
