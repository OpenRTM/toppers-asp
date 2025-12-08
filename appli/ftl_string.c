#include "tlsf.h"
#include "string.h"

char* strdup(const char* s) {
    if (s == NULL) return NULL;

    size_t len = strlen(s) + 1;
    char* copy = (char*)tlsf_malloc(len);
    if (copy == NULL) return NULL;

    memcpy(copy, s, len-1);
    copy[len-1] = 0;
    return copy;
}

char* strndup(const char* s,size_t n) {
    if (s == NULL) return NULL;

    size_t len = strnlen(s,n) + 1;
    char* copy = (char*)tlsf_malloc(len);
    if (copy == NULL) return NULL;

    memcpy(copy, s, len-1);
    copy[len-1] = 0;
    return copy;
}
