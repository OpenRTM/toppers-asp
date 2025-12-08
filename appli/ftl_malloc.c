
#include "tlsf.h"
#include <reent.h>

void* _malloc_r(struct _reent* r, size_t size) {
    (void)r;
    return tlsf_malloc(size);
}

void _free_r(struct _reent* r, void* ptr) {
    (void)r;
    tlsf_free(ptr);
}

void* _realloc_r(struct _reent* r, void* ptr, size_t size) {
    (void)r;
    return tlsf_realloc(ptr, size);
}

void* _calloc_r(struct _reent* r, size_t num, size_t size) {
    (void)r;
    void* ptr = tlsf_malloc(num * size);
    if (ptr) memset(ptr, 0, num * size);
    return ptr;
}
