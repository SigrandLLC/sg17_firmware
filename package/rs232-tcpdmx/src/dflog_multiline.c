#define _GNU_SOURCE
#include <dflog.h>
#include <dferror.h>
#include <string.h>     // strchr

void dflog_multiline(int level, const char *pfx, const char *s, size_t n)
{
    char *ss = (n != 0) ? strndup(s, n) : strdup(s);
    if (ss == NULL)
    {
	dferror(EXIT_SUCCESS, errno, "Can't allocate %zu bytes", (n != 0) ? n : strlen(s)+1);
	ss = (char *)s; // Yes, I know
    }

    const char *str = ss;
    char *nl;
    size_t count = 0;
    while ( (nl = strchr(str, '\n')) )
    {
	*nl = '\0';
	dflog(level, "%s %zu %s", pfx, count, str);
	*nl = '\n';

	str = ++nl;
	++count;
    }
    if (strlen(str) != 0)
	dflog(level, "%s %zu %s", pfx, count, str);
}
