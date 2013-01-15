#include <x.h>
#include <string.h> // memset

void *xzmalloc(size_t size)
{
    void *ret = xmalloc(size);
    memset(ret, 0, size);
    return ret;
}
