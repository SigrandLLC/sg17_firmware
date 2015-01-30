#include <stdio.h>

static void prchar(int c)
{
    if (c < 040 || c == 0177)
	printf("^%c", c + 0100);
    else
	printf(" %c", c);
}

static void prchar2(int c)
{
    printf("'"); prchar(c); printf("' : %04o", c);
}

static void prchar2nl(int c)
{
    prchar2(c); printf("'\r\n");
    fflush(stdout);
}

static void prchar2pfx_nl(const char *pfx, int c)
{
    printf(pfx); prchar2nl(c);
}

