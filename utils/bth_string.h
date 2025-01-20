// MIT License
// 
// Copyright (c) 2025 bobthehuge
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

#ifndef BTH_STRING_H
#define BTH_STRING_H

#include <stdlib.h>

#ifndef BTH_STRING_ALLOC
#define BTH_STRING_ALLOC(n) malloc(n)
#endif

#ifdef BTH_STRING_IMPLEMENTATION

// get n '\n' terminated string from buf
//
// returns char pointer array to each beginning of line
// if `n` points to nothing, return all lines
// else get all lines and put the line count to the pointed `n`
//
char **getnlines(char *_buf, size_t *_n)
{
    char *buf = _buf;
    size_t n = _n ? *_n : 0;
    char c = 0;

    if (!n)
        while ((c = *buf++))
            if (c == '\n')
                n++;

    buf = _buf;

    char **lines = BTH_STRING_ALLOC(n * sizeof(char *));

    if (!lines)
        return NULL;

    lines[0] = buf;
    size_t n2 = 1;

    while (n2 <= n && (c = *buf++))
        if (c == '\n')
        {
            lines[n2] = buf;
            n2++;
        }

    if (_n)
        *_n = n2;

    return lines;
}

// searches `c` from `end` to `start`. Doesn't check for `end` > `start`
// 
// returns `start` if `c` isn't found
// 
char *findrchr(char *_end, char *start, int chr)
{
    char *end = _end;
    int c = 0;

    while (end > start && *end-- != chr) {}
    return end;
}

#endif

#endif
