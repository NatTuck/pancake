
#include <stddef.h>
#include <drip/lstring.h>

#include "pancake/cache.h"

pancake_kernel_cache*
pancake_cache_add(pancake_kernel_cache* cc, char* key, cl_kernel kk)
{
    pancake_kernel_cache* yy = GC_malloc(sizeof(pancake_kernel_cache));
    yy->key    = lstrdup(key);
    yy->kernel = kk;
    yy->next   = cc;
    return yy;
}

cl_kernel
pancake_cache_get(pancake_kernel_cache* cc, char* key)
{
    if (cc == 0)
        return 0;

    if (streq(cc->key, key))
        return cc->kernel;

    return pancake_cache_get(cc->next, key);
}
