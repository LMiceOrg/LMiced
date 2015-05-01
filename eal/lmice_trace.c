#include "lmice_trace.h"


#if defined(_DEBUG)
const int lmice_trace_debug_mode = 1;
#else
const int lmice_trace_debug_mode = 0;
#endif

#if defined(_WIN32)

lmice_trace_name_t lmice_trace_name[] =
{
    {lmice_trace_info,      "INFO",     10 /* light_green*/},
    {lmice_trace_debug,     "DEBUG",    11 /* light_cyan */},
    {lmice_trace_warning,   "WARNING",  14 /*yellow*/},
    {lmice_trace_error,     "ERROR",    12 /*light_red*/},
    {lmice_trace_critical,  "CRITICAL", 13 /* light_purple*/},
    {lmice_trace_none,      "NULL",     7 /* white */}
};
#else

lmice_trace_name_t lmice_trace_name[] =
{
    {lmice_trace_info,      "INFO",     "\033[1;32m" /* light_green*/},
    {lmice_trace_debug,     "DEBUG",    "\033[1;36m" /* light_cyan */},
    {lmice_trace_warning,   "WARNING",  "\033[1;33m" /*yellow*/},
    {lmice_trace_error,     "ERROR",    "\033[1;31m" /*light_red*/},
    {lmice_trace_critical,  "CRITICAL", "\033[1;35m" /* light_purple*/},
    {lmice_trace_none,"NULL","\033[0m"}
};

#endif
