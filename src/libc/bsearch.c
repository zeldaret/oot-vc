#include "revolution/types.h"

void* bsearch(const void* key, const void* base, size_t nmemb, size_t size, int (*compar)(const void*, const void*)) {
    const void* p;
    size_t lower;
    size_t upper;
    size_t index;
    int cmp;

    if (key == NULL || base == NULL || nmemb == 0 || size == 0 || compar == NULL)
        return NULL;

    p = base;
    cmp = (*compar)(key, p);
    if (cmp == 0)
        return (void*)p;

    if (cmp < 0) {
        return NULL;
    }

    upper = nmemb - 1;
    lower = 1;
    while (lower <= upper) {
        index = lower + upper >> 1;
        p = (const char*)base + size * index;
        cmp = (*compar)(key, p);
        if (cmp == 0)
            return (void*)p;
        if (cmp < 0)
            upper = index - 1;
        else {
            lower = index + 1;
        }
    }

    return (NULL);
}
