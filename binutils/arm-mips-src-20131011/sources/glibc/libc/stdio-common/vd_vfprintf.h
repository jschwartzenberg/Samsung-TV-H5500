#include <stdio.h>
#include <stdarg.h>

/* Native instance of fprintf */
extern int fprintf_native (FILE *stream, const char *format, ...);

/* Native instance of printf */
extern int printf_native (const char *format, ...);

/* Native instance of vfprintf */
extern int vfprintf_native (FILE *s, const char *format, va_list ap);
