#include <juve_utils.h>
#include <string.h>
#include <stdio.h>
#include <stdarg.h>

#define CBUF_INI_CAP 255

struct CJBuffer {
    char* data;
    size_t len;
    size_t cap;

    JArena* arena;
};


CJBuffer* cjb_create(JArena* arena) {
    CJBuffer* cbuf = (CJBuffer*)jarena_alloc(arena, sizeof(CJBuffer));
    if (!cbuf) return NULL;

    cbuf->len = 0;
    cbuf->cap = CBUF_INI_CAP;
    cbuf->arena = arena;
    
    cbuf->data = (char*)jarena_alloc(arena, sizeof(char)*cbuf->cap);
    if (!cbuf->data) return NULL;
    return cbuf;
}

CJBuffer* cjb_from_str(JArena* arena, const char* cstr) {
    CJBuffer* cbuf = cjb_create(arena);
    cjb_append(cbuf, cstr);
    return cbuf;
}

size_t cjb_append(CJBuffer* cbuf, const char* str) {
    if (strlen(str) == 0) return 0;
    
    size_t total_len = cbuf->len + strlen(str);
    if (total_len >= cbuf->cap) {
        size_t newsz = total_len > cbuf->cap*2 ? total_len : cbuf->cap*2;
        char* new_data = (char*)jarena_alloc(cbuf->arena, newsz);
        if (!new_data) return -1;      
        
        memcpy(new_data, cbuf->data, sizeof(char)*cbuf->len);
        cbuf->data = new_data;
        cbuf->cap = newsz;
    }

    memcpy(cbuf->data + cbuf->len, str, strlen(str) + 1);
    cbuf->len =  total_len;
    return total_len;
}

size_t cjb_appendf(CJBuffer* cbuf, const char* fmt, ...) {
    if (strlen(fmt) == 0) return 0;

    va_list args;
    va_start(args, fmt);

    size_t needed = vsnprintf(NULL, 0, fmt, args);

    char* str = (char*)jarena_alloc(cbuf->arena, needed + 1);
    va_list args_real;
    va_start(args_real, fmt);

    vsnprintf(str, needed + 1, fmt, args_real);

    cjb_append(cbuf, str);
    
    return 1;
}

void  cjb_print(CJBuffer* cbuf) {
    printf("%s", cbuf->data);
}

char* cjb_str(CJBuffer* cbuf) {
    return jarena_strdup(cbuf->arena, cbuf->data);
}

void  cjb_clear(CJBuffer* cbuf) { cbuf->len = 0; cbuf->data[0] = '\0'; }
bool  cjb_eq(CJBuffer* cbuf, const char* str) { return strcmp(cbuf->data, str) == 0; }

size_t cjb_len(CJBuffer* cbuf) { return cbuf->len; }
