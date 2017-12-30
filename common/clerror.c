#include <stdio.h>
#include <stdarg.h>

static char lastError[1024] = "no error";

char *getLastCLError()
{
    return lastError;
}

void setLastCLError(char *fmt, ...)
{
    va_list ap;

    va_start(ap, fmt);
    vsnprintf(lastError, sizeof lastError, fmt, ap);
    va_end(ap);
}
