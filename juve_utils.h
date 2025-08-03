#pragma once
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <stdio.h>

/**
 * Juve Library
 * Contains implementations for various utilities that i frequently use
 **/


#include <stddef.h>
#include <stdbool.h>

#define JBUF_INI_CAP 255
#define JARENA_INI_CAP 255

#define MAX(a, b) ((a > b) ? a : b)

typedef struct JArena JArena;
struct JArena {
    char* base;
    size_t capacity, offset;
};

static inline JArena  jarena_new(void);
static inline void*   jarena_alloc(JArena*, size_t);
static inline void*   jarena_zeroed(JArena*, size_t);
static inline void    jarena_free(JArena*);
static inline void    jarena_reset(JArena*);
static inline char*   jarena_strdup(JArena*, char*);

#ifdef JUVE_IMPLEMENTATION
static inline JArena jarena_new() {
    JArena arena;
    arena.capacity = JARENA_INI_CAP;
    arena.offset   = 0;
    arena.base     = (char*)malloc(arena.capacity);
    return arena;
}

static inline void jarena_reset(JArena* arena) {
    arena->offset = 0;
}

static inline void* jarena_alloc(JArena* arena, size_t size) {
    const size_t align = 8;
    size = (size + (align - 1)) & ~(align - 1);

    if (arena->offset + size > arena->capacity) {
        size_t new_size = MAX(arena->offset + size, arena->capacity*2);
        arena->base = realloc(arena->base, new_size);
        arena->capacity = new_size;
    }
    
    void* ptr = arena->base + arena->offset;
    arena->offset += size;
    return ptr;
}

static inline void*  jarena_zeroed(JArena* arena, size_t size) {
    void* ptr = jarena_alloc(arena, size);
    if (ptr) memset(ptr, 0, size);
    return ptr;
}

static inline char* jarena_strdup(JArena* arena, char* str) {
    size_t len = strlen(str) + 1;
    char* copy = (char*)jarena_alloc(arena, len);
    if (copy) memcpy(copy, str, len);
    return copy;
}
#endif


// JBuffer: A resizable buffer for string operations
typedef struct JBuffer JBuffer;
struct JBuffer {
    char* data;
    size_t len;
    size_t cap;

    JArena* arena;
};

JBuffer     jb_create(JArena*);
size_t        jb_append(JBuffer*, const char*);
void        jb_appendf_a(JBuffer*, JArena*, const char* fmt, ...);
void        jb_print(JBuffer);
char*       jb_str(JBuffer);
char*       jb_str_a(JBuffer jb, JArena* arena);
void        jb_free(JBuffer*);
void        jb_clear(JBuffer*);
bool        jb_eq(JBuffer, const char*);
size_t      jb_len(JBuffer);

#ifdef JUVE_IMPLEMENTATION
JBuffer jb_create(JArena* arena) {
    return (JBuffer) {
        .len = 0,
        .cap = JBUF_INI_CAP,
        .data = jarena_alloc(arena, sizeof(char)*JBUF_INI_CAP),
        .arena = arena
    };
}

JBuffer jb_from_str(JArena* arena, const char* cstr) {
    JBuffer buf = jb_create(arena);
    jb_append(&buf, cstr);
    return buf;
}

size_t jb_append(JBuffer* buf, const char* str) {
    if (strlen(str) == 0) return 0;
    
    size_t total_len = buf->len + strlen(str);
    if (total_len >= buf->cap) {
        size_t newsz = total_len > buf->cap*2 ? total_len : buf->cap*2;
        char* new_data = (char*)jarena_alloc(buf->arena, newsz);
        if (!new_data) return -1;      
        
        memcpy(new_data, buf->data, sizeof(char)*buf->len);
        buf->data = new_data;
        buf->cap = newsz;
    }

    memcpy(buf->data + buf->len, str, strlen(str) + 1);
    buf->len =  total_len;
    return total_len;
}

size_t jb_appendf(JBuffer* buf, const char* fmt, ...) {
    if (strlen(fmt) == 0) return 0;

    va_list args;
    va_start(args, fmt);

    size_t needed = vsnprintf(NULL, 0, fmt, args);

    char* str = (char*)jarena_alloc(buf->arena, needed + 1);
    va_list args_real;
    va_start(args_real, fmt);

    vsnprintf(str, needed + 1, fmt, args_real);

    jb_append(buf, str);
    
    return 1;
}

void  jb_print(JBuffer buf) {
    printf("%s", buf.data);
}

char* jb_str(JBuffer buf) {
    return jarena_strdup(buf.arena, buf.data);
}

void  jb_clear(JBuffer* buf) { buf->len = 0; buf->data[0] = '\0'; }
bool  jb_eq(JBuffer buf, const char* str) { return strcmp(buf.data, str) == 0; }
size_t jb_len(JBuffer buf) { return buf.len; }
#endif


size_t jb_read_entire_file(const char*, JBuffer*);
bool   jfile_write(const char*, const char*);
char*  jfile_stem(const char*, JArena*);
char*  jfile_ext(const char*, JArena*);
char*  jfile_dirname(const char*, JArena*);
char*  jfile_basename(const char*, JArena*);
bool   jfile_exists(const char*);
size_t jfile_size(const char*);


#ifdef JUVE_IMPLEMENTATION
size_t jfile_size(const char* path) {
    FILE* file = fopen(path, "r");
    if (!file) return -1;

    fseek(file, 0, SEEK_END);
    size_t size = ftell(file);
    
    fclose(file);
    return size;
}

size_t jb_read_entire_file(const char* path, JBuffer* buffer) {
    size_t size = jfile_size(path);
    if (size == -1) return -1;

    FILE* file = fopen(path, "rb");
    char* content = (char*)jarena_alloc(buffer->arena, size + 1);
    if (!content) return -1;
    
    fread(content, 1, size, file);
    if (!content) return -1;

    jb_append(buffer, content);
    
    fclose(file);
    return size;
}

bool jfile_write(const char* path, const char* content) {
    FILE* file = fopen(path, "wb");
    if (!file) return false;

    fprintf(file, "%s", content);
    fclose(file);
    return true;
}

char* jfile_stem(const char* path, JArena* arena) {
    const char* path_ = path;
    const char* last_slash = strrchr(path, '/');

    if (last_slash) {
        path_ = last_slash + 1;
    }

    const char* last_dot = strrchr(path_, '.');
    size_t len = last_dot ? (size_t) (last_dot - path_) : strlen(path_);
    char* stem = (char*)jarena_alloc(arena, len + 1);

    if (!stem) return NULL;
    memcpy(stem, path_, len);
    stem[len] = '\0';
    return stem;
}

char* jfile_ext(const char* path, JArena* arena) {
    const char* filename = path;
    const char* last_dot = strrchr(path, '.');

    if (last_dot) {
        char* ext = jarena_alloc(arena, strlen(last_dot));
        memcpy(ext, last_dot + 1, strlen(last_dot));
        return ext;
    }
    return NULL;
}

char* jfile_dirname(const char* path, JArena* arena) {
    
}

#endif


// containers
typedef struct JVec JVec;
JVec*       jvec_new(void);
JVec*       jvec_lines(const char* source, JArena*);
void        jvec_push(JVec*, void*);
size_t      jvec_len(JVec*);
void*       jvec_at(JVec*, size_t);
void*       jvec_back(JVec*);
void        jvec_free(JVec*);

//==------------------- Pure Implemntation -------------------===

typedef struct CJArena CJArena;

typedef struct CJVec CJVec;
CJVec*      cjvec_new(JArena* arena);
CJVec*      cjvec_lines(void);
void        cjvec_push(CJVec* vec, void* data);
size_t      cjvec_len(CJVec*);
void*       cjvec_at(CJVec*, size_t);
void*       cjvec_back(CJVec*);
void        cjvec_free(CJVec*);

typedef struct CJBuffer CJBuffer;
CJBuffer*   cjb_create(JArena* arena);
CJBuffer*   cjb_from_str(JArena* arena, const char* cstr);
size_t      cjb_append(CJBuffer*, const char*);
size_t      cjb_appendf(CJBuffer*, const char* fmt, ...);
void        cjb_print(CJBuffer*);
char*       cjb_str(CJBuffer*);
void        cjb_clear(CJBuffer*);
bool        cjb_eq(CJBuffer*, const char*);
size_t      cjb_len(CJBuffer*);

typedef struct CJMap CJMap;
CJMap*      cjmap_create(JArena* arena);
bool        cjmap_put(CJMap* map, const char* key, void* value);
bool        cjmap_has(CJMap* map, const char* key);
void*       cjmap_get(CJMap* map, const char* key);
bool        cjmap_remove(CJMap* map, const char* key);


typedef struct CJFile CJFile;

typedef enum {
    JCMD_LOG,
    JCMD_NOT_SET,
} JCmdOption;

typedef struct CJCmd {
    JCmdOption opt;
    CJBuffer* buffer;
} CJCmd;

#define cmd_append(cmd, ...) \
        cmd_append_many((cmd), \
                 ((const char*[]) {__VA_ARGS__}), (sizeof((const char*[]) {__VA_ARGS__})/sizeof(const char*)))

#define cmd_append_many(cmd, items, item_count) \
       do {\
           for (size_t i = 0; i < (item_count); ++i) {\
              const char* atom = (items)[i];\
              jcmd_append((cmd), atom);\
           }\
       } while (0)

       
CJCmd jcmd_init(JArena* arena, JCmdOption opt);
bool jcmd_run(CJCmd cmd);
bool jcmd_one_shot(CJCmd* cmd);
void jcmd_reset(CJCmd* cmd);
void jcmd_append(CJCmd* cmd, const char* atom);

#ifdef __cplusplus
}
#endif
