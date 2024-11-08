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


#ifndef NOLOG
#define LOG(...) \
{\
    fprintf(stderr, "[LOG] ");\
    warn(__VA_ARGS__);\
}

#define LOGX(...) \
{\
    fprintf(stderr, "[LOG] ");\
    warnx(__VA_ARGS__);\
}
#endif

#ifndef NOWARN
#define WARN(...) \
{\
    fprintf(\
        stderr,\
        "[WARNING] at '%s':'%s': %d\n-> ",\
        __FILE__, __func__, __LINE__);\
    warn(__VA_ARGS__);\
}

#define WARNX(...) \
{\
    fprintf(\
        stderr,\
        "[WARNING] at '%s':'%s': %d\n-> ",\
        __FILE__, __func__, __LINE__);\
    warnx(__VA_ARGS__);\
}
#endif

#ifndef NOERR
#define ERR(...) \
{\
    fprintf(\
        stderr,\
        "[FATAL] at '%s':'%s': %d\n-> ",\
        __FILE__, __func__, __LINE__\
    );\
    err(__VA_ARGS__);\
}

#define ERRX(...) \
{\
    fprintf(\
        stderr,\
        "[FATAL] at '%s':'%s': %d\n-> ",\
        __FILE__, __func__, __LINE__);\
    errx(__VA_ARGS__);\
}
#endif

#ifndef NOTODO
#define TODOX(...) \
{\
    fprintf(\
        stderr,\
        "[TODO] at '%s':'%s': %d\n-> ",\
        __FILE__, __func__, __LINE__);\
    errx(__VA_ARGS__);\
}

#define TODO() TODOX(1, "todo!")
#endif

#endif /* ! */
