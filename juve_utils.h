#pragma once
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdint.h>

/**
 * Juve Library
 * Contains implementations for various utilities that i frequently use
 **/


#include <stddef.h>
#include <stdbool.h>

#define JBUF_INI_CAP 255
#define JARENA_INI_CAP 255

#define MAX(a, b) ((a > b) ? a : b)

/* Tracker Arena Implementation
 * 
 * Doesn't use Linear allocations.
 *       JArena allocates with 'malloc' provided by libc and stores the pointer returned
 * on free, the pointer array is iterated and each allocated pointer is freed
*/
typedef struct JArena JArena;
struct JArena {
    void** data;
    size_t capacity, count;
};

JArena  jarena_new(void);
void*   jarena_alloc(JArena*, size_t);
void*   jarena_zeroed(JArena*, size_t);
void    jarena_free(JArena*);
void    jarena_reset(JArena*);
char*   jarena_strdup(JArena*, char*);

#ifdef JUVE_IMPLEMENTATION
JArena jarena_new() {
    JArena arena;
    arena.capacity = JARENA_INI_CAP;
    arena.count   = 0;
    arena.data     = (void**)malloc(arena.capacity * sizeof(void*));
    return arena;
}

void jarena_reset(JArena* arena) {
    arena->count = 0;
}

// Allocates 'size' bytes and stores the pointer in 'arena'
void* jarena_alloc(JArena* arena, size_t size) {
    void* ptr = malloc(size);
    if (!ptr) return NULL;

    if (arena->count >= arena->capacity) {
        size_t new_sz = arena->capacity * 2;
        void** new_data = malloc(new_sz * sizeof(void*));
        if (!new_data) {
            free(ptr);
            return NULL;
        }
        
        memcpy(new_data, arena->data, arena->count*sizeof(void*));
        free(arena->data);
        arena->data = new_data;
        arena->capacity = new_sz;
    }
    
    arena->data[arena->count++] = ptr;
    return ptr;
}

// Allocates 'size' bytes in 'arena' and zeroes the allocated memory
void*  jarena_zeroed(JArena* arena, size_t size) {
    void* ptr = jarena_alloc(arena, size);
    if (ptr) memset(ptr, 0, size);
    return ptr;
}

void jarena_free(JArena* arena) {
    if (arena) {
        for (int i = 0; i < arena->count; i++) {
            free(arena->data[i]);
        }
        free(arena->data);
    }
}

// Copies 'str' into 'arena'
char* jarena_strdup(JArena* arena, char* str) {
    if (!str) return NULL;
    
    size_t len = strlen(str) + 1;
    char* copy = (char*)jarena_alloc(arena, len);
    if (!copy) return NULL;

    memcpy(copy, str, len);
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

JBuffer jb_create(JArena*);
size_t  jb_append(JBuffer*, const char*);
size_t  jb_appendf(JBuffer*, const char* fmt, ...);
void    jb_print(JBuffer);
char*   jb_str(JBuffer);
char*   jb_str_a(JBuffer jb, JArena* arena);
void    jb_free(JBuffer*);
void    jb_clear(JBuffer*);
bool    jb_eq(JBuffer, const char*);
size_t  jb_len(JBuffer);

#ifdef JUVE_IMPLEMENTATION
JBuffer jb_create(JArena* arena) {
    JBuffer buf = {0};
    buf.len = 0;
    buf.cap = JBUF_INI_CAP;
    buf.data = jarena_alloc(arena, sizeof(char)*JBUF_INI_CAP);
    buf.arena = arena;
    return buf;
}

JBuffer jb_from_str(JArena* arena, const char* cstr) {
    JBuffer buf = jb_create(arena);
    jb_append(&buf, cstr);
    return buf;
}

size_t jb_append(JBuffer* buf, const char* str) {
    if (!str || !buf) return -1;
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
    if (!fmt || fmt[0] == '\0') return 0;

    va_list args;
    va_start(args, fmt);

    va_list copy;
    va_copy(copy, args);

    int n = vsnprintf(NULL, 0, fmt, args);
    if (n < 0) {
        va_end(copy);
        return 0;
    }
    size_t needed = (size_t)n;
    va_end(args);

    if (needed == 0) {
        va_end(copy);
        return 0;
    }
    
    char* str = (char*)jarena_alloc(buf->arena, needed + 1);
    if (!str) {
        va_end(copy);
        return 0;
    }
    
    vsnprintf(str, needed + 1, fmt, copy);
    va_end(copy);
    
    jb_append(buf, str);    
    return needed;
}

void  jb_print(JBuffer buf) {
    printf("%s", buf.data);
}

char* jb_str(JBuffer buf) {

    if (!buf.data) return NULL;
    
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
char*  jfile_basename(const char*, JArena*);
bool   jfile_exists(const char*);
size_t jfile_size(const char*);

// char*  jfile_dirname(const char*, JArena*);

#ifdef JUVE_IMPLEMENTATION
size_t jfile_size(const char* path) {
    FILE* file = fopen(path, "r");
    if (!file) return -1;

    fseek(file, 0, SEEK_END);
    size_t size = ftell(file);
    
    fclose(file);
    return size;
}


// Read the content of file at 'path' into 'buffer'
size_t jb_read_entire_file(const char* path, JBuffer* buffer) {
    if (!path || !buffer) return -1;
    
    size_t size = jfile_size(path);
    if (size == (size_t)-1) return -1;

    FILE* file = fopen(path, "rb");
    if (!file) return -1;
    
    char* content = (char*)jarena_alloc(buffer->arena, size + 1);
    if (!content) return -1;
    
    size_t read = fread(content, 1, size, file);
    if (read != size) {
        fclose(file);
        return (size_t)-1;
    }
    content[size] = '\0';
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

// Returns a stripped version of filename at 'path'
// not containing either the extension or parent directory
//
// if 'foo/bar/baz.txt' was passed, then 'baz' is returned
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

// Returns the extension of 'path'
//
// if 'foo.txt' is passed, then 'txt' is returned without the '.'
char* jfile_ext(const char* path, JArena* arena) {
    const char* filename = path;
    const char* last_dot = strrchr(path, '.');

    if (last_dot) {
        return jarena_strdup(arena, (char*)last_dot + 1);
    }
    return NULL;
}

// Returns the filename at 'path' stripping off any parent directory
// 
// if 'foo/bar/baz.txt' is passed, then 'baz.txt' is returned
char* jfile_basename(const char* path, JArena* arena) {
    const char* filename = path;
    const char* last_slash = strrchr(path, '/');
    
    if (last_slash) {
        return jarena_strdup(arena, (char*)last_slash + 1);
    }
    
    return (char*)filename;
}

bool jfile_exists(const char* path) {
    FILE* f = NULL;
    return ((f = fopen(path, "r")) != NULL);
}
#endif


#define CJVEC_INIT 255
#define MAX_BUCKETS 35

// JVec: A Dynamic array of 'void*'
typedef struct JVec JVec;
struct JVec {
    void** data;
    size_t cap;
    size_t len;
    JArena* arena;
};

JVec   jvec_lines(char* source, JArena*);
JVec   jvec_new(JArena* arena);
void   jvec_push(JVec* vec, void* data);
size_t jvec_len(JVec);
void*  jvec_at(JVec*, size_t);
void*  jvec_back(JVec*);
void   jvec_free(JVec*);

// JMap: A HashMap that maps a 'const char*' -> 'void*'
typedef struct JMap JMap;
typedef struct Bucket_ {
    const char* key;
    void* value;
    struct Bucket_* next;
} Bucket;

struct JMap {
    Bucket* buckets[MAX_BUCKETS];
    JArena* arena;
};
JMap  jmap_create(JArena* arena);
bool  jmap_put(JMap* map, const char* key, void* value);
bool  jmap_has(JMap map, const char* key);
void* jmap_get(JMap map, const char* key);
bool  jmap_remove(JMap* map, const char* key);


#ifdef JUVE_IMPLEMENTATION
JVec jvec_new(JArena* arena) {
    JVec vec = {0};
    
    vec.cap = CJVEC_INIT;
    vec.len = 0;
    vec.arena = arena;

    vec.data = (void**)jarena_alloc(arena, sizeof(void*)*vec.cap);    
    return vec;
}

// Returns a vector containing each line in 'source'.
// all lines have been copied into the arena
//
// int main(void) {
//      const char* source = "foo\nbar\nbaz";
//      JVec vec = jvec_lines(source, &arena);
//
//      assert(strcmp(jvec_at(&vec, 0), "foo") == 0);
//      assert(strcmp(jvec_at(&vec, 1), "bar") == 0);
//      assert(strcmp(jvec_at(&vec, 2), "baz") == 0);
// }
//
JVec jvec_lines(char* source, JArena* arena) {
    char* content = source;
    JVec vec = jvec_new(arena);

    const char* line_start = source;
    const char* ptr = source;

    while (*ptr) {
        if (*ptr == '\n' || *ptr == '\r') {
            const char* line_end = ptr;
            if (*ptr == '\r' && *(ptr + 1) == '\n') ptr++;

            size_t len = (size_t)(line_end - line_start);
            char*  line = jarena_strdup(arena, (char*)line_start);
            line[len] = '\0';
            jvec_push(&vec, line);
            ptr++;
            line_start = ptr;
        } else {
            ptr++;
        }
    }

    if (ptr != line_start) {
        size_t len = (size_t)(ptr - line_start);
        char* line = jarena_strdup(arena, (char*)line_start);
        line[len] = '\0';
        jvec_push(&vec, line);
    }
    
    return vec;
}

//
// Appends 'data' to the end of 'vec'
//
// int main(void) {
//     JVec vec = jvec_new(&arena);
//     jvec_push(&vec, "foo");
//     jvec_push(&vec, "bar");
//
//     assert(strcmp(jvec_at(&vec, 0), "foo") == 0);
//     assert(strcmp(jvec_at(&vec, 1), "bar") == 0);
// }
void jvec_push(JVec* vec, void* data) {
    if (vec->len >= vec->cap) {
        size_t newsz = vec->cap*2;
        void** new_data = (void**)jarena_alloc(vec->arena, newsz);       
        
        memcpy(new_data, vec->data, vec->len);
        vec->data = new_data;
        vec->cap = newsz;
    }
    vec->data[vec->len++] = data;
}

size_t jvec_len(JVec vec) {
    return vec.len;
}

// Returns the element in 'vec' that is at index 'n'
void* jvec_at(JVec* vec, size_t n) {
    if (!vec) return NULL;
    return vec->data[n];
}

void* jvec_back(JVec* vec) {
    if (!vec) return NULL;
    return vec->data[vec->len-1];    
}

void jvec_free(JVec* vec) {
    (void) vec;
    // don't free anything.
    // structure is allocated on the arena
}


uint32_t chash(const char* str) {    
    uint32_t hash = 5381;
    int c;

    while ((c = *str++))
        hash = ((hash << 5) + hash) + c; /* hash * 33 + c */

    return hash;
}

JMap jmap_create(JArena* arena) {
    JMap map = {0};
    memset(map.buckets, 0, sizeof(map.buckets));
    map.arena = arena;
    return map;
}

bool jmap_put(JMap* map, const char* key, void* value) {
    if (!map || !key) return false;
    
    uint32_t index = chash(key) % MAX_BUCKETS;

    Bucket* current = map->buckets[index];
    Bucket* prev    = NULL;
    
    while (current) {
        if (strcmp(current->key, key) == 0) {
            current->value = value;
            return true;
        }
        prev    = current;
        current = current->next;
    }

    Bucket* new_buck = jarena_alloc(map->arena, sizeof(Bucket));
    new_buck->key   = key;
    new_buck->value = value;
    new_buck->next  = NULL;
    if (prev) prev->next = new_buck;
    else map->buckets[index] = new_buck;
    return true;
}

  bool jmap_has(JMap map, const char* key) {
    uint32_t index = chash(key) % MAX_BUCKETS;
    Bucket*  current = map.buckets[index];
    while (current) {
        if (strcmp(current->key, key) == 0) return true;
        current = current->next;
    }
    return false;
}

bool jmap_remove(JMap* map, const char* key) {
    uint32_t index = chash(key) % MAX_BUCKETS;
    Bucket*  current = map->buckets[index];
    while (current) {
        if (strcmp(current->key, key) == 0) {
            current = NULL;
            // found the key
            return true;
        }
        current = current->next;
    }
    
    // we could not find the key
    return false;
}

void* jmap_get(JMap map, const char* key) {
    uint32_t index = chash(key) % MAX_BUCKETS;
    Bucket* current = map.buckets[index];
    while (current) {
        if (strcmp(current->key, key) == 0) return current->value;
        current = current->next;
    }
    return NULL;
}
#endif

typedef enum {
    JCMD_LOG,
    JCMD_NOT_SET,
} JCmdOption;

typedef struct CJCmd {
    JCmdOption opt;
    JBuffer buffer;
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

#ifdef JUVE_IMPLEMENTATION
CJCmd jcmd_init(JArena* arena, JCmdOption opt) {
    return (CJCmd) {
        .buffer = jb_create(arena),
        .opt = opt
    };
}

bool jcmd_run(CJCmd cmd) {
    const char* str = jb_str(cmd.buffer);
    if (cmd.opt == JCMD_LOG) {
        printf("Running Command: %s\n", str);
    }
    return system(str);
}

void jcmd_append(CJCmd* cmd, const char* atom) {
    jb_appendf(&cmd->buffer, "%s ", atom);
}

bool jcmd_one_shot(CJCmd* cmd) {
    bool r = jcmd_run(*cmd);
    jb_clear(&cmd->buffer);
    return r;
}

void jcmd_reset(CJCmd* cmd) {
    jb_clear(&cmd->buffer);    
}
#endif
