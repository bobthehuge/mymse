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

#ifndef BTH_LOG_H 
#define BTH_LOG_H

#include <err.h>
#include <stdio.h>
#include <stdlib.h>

#ifndef BTH_LOG_BUF_LEN
#define BTH_LOG_BUF_LEN 1024
#endif

#ifndef BTH_LOG_BUF_DECL
#define BTH_LOG_BUF_DECL \
    static char __bth_log_buf[BTH_LOG_BUF_LEN] = {0};
#endif

#ifndef BTH_LOG_TXT_FMT
#define BTH_LOG_TXT_FMT(fmt, ...) \
    static char __bth_log_buf[BTH_LOG_BUF_LEN] = {0};\
    memset(__bth_log_buf, 0, BTH_LOG_BUF_LEN);\
    int ch = snprintf(__bth_log_buf, BTH_LOG_BUF_LEN, fmt, __VA_ARGS__);\
    if (ch > BTH_LOG_BUF_LEN) {\
        char *ptr = __bth_log_buf + BTH_LOG_BUF_LEN - 4;\
        sprintf(ptr, "...");\
    }
#endif

#ifndef NOLOG
#define LOG(fmt, ...) \
{\
    BTH_LOG_TXT_FMT(fmt, __VA_ARGS__);\
    fprintf(stderr, "[LOG] ");\
    warnx("%s : %s : %d => %s",\
        __FILE__, __func__, __LINE__, __bth_log_buf);\
}
#endif

#ifndef NOWARN
#define WARN(fmt, ...) \
{\
    BTH_LOG_TXT_FMT(fmt, __VA_ARGS__);\
    fprintf(stderr, "[WARN] ");\
    warn("%s : %s : %d => %s",\
        __FILE__, __func__, __LINE__, __bth_log_buf);\
}

#define WARNX(fmt, ...) \
{\
    BTH_LOG_TXT_FMT(fmt, __VA_ARGS__);\
    fprintf(stderr, "[WARN] ");\
    warnx("%s : %s : %d => %s",\
        __FILE__, __func__, __LINE__, __bth_log_buf);\
}
#endif

#ifndef NOERR
#define ERR(code, fmt, ...) \
{\
    BTH_LOG_TXT_FMT(fmt, __VA_ARGS__);\
    fprintf(stderr, "[FATAL] ");\
    err(code, "%s : %s : %d => %s",\
        __FILE__, __func__, __LINE__, __bth_log_buf);\
}

#define ERRX(code, fmt, ...) \
{\
    BTH_LOG_TXT_FMT(fmt, __VA_ARGS__);\
    fprintf(stderr, "[FATAL] ");\
    errx(code, "%s : %s : %d => %s",\
        __FILE__, __func__, __LINE__, __bth_log_buf);\
}

#endif

#ifndef NOTODO
#define TODO() ERRX(1, "%s", "todo!")
#endif

#ifndef NOTRACE
#define TRACE(label, fmt, ...) \
{\
    BTH_LOG_TXT_FMT(fmt, __VA_ARGS__);\
    fprintf(stderr, "%s ", label);\
    warnx("%s : %s : %d => %s",\
        __FILE__, __func__, __LINE__, __bth_log_buf);\
}
#endif

#endif /* ! */
