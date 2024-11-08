#ifndef CML_LEXER_H
#define CML_LEXER_H

#include "cml_token.h"

#include "../utils/bth_dynarray.h"

struct cml_lexer
{
    char *buf_path;
    char *buf;
    uint32_t buf_len;
    uint32_t pos;
    uint32_t read_pos;
    uint32_t row;
    uint32_t col;
    char ch;
};

int cml_lexer_from_file(struct cml_lexer *lex, char *path);
void cml_lexer_destroy(struct cml_lexer *lex);
struct cml_token cml_lexer_next_token(struct cml_lexer *lex);
struct bth_dynarray cml_lexer_lexall(struct cml_lexer *lex);

#endif
