#pragma once


/*
* Juve Library
* Contains implementations for various utilities that i frequently use
* it is split into two versions:
    * STL-Implementation: this version uses C++ stdlib to provide support for these utilites
    * Pure Implementation: this comprises of the utilites that are implemented in pure C .
**/

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>
#include <stdbool.h>
//==------------------- STL-Implemntation -------------------===

// JBuffer: A resizable buffer for string operations (non arena)
// Uses std::stringstream under the hood
typedef struct JBuffer JBuffer;


// JArena: A basic allocation tracker.
// stores allocations in one place ready to be torn down in one go
// Uses std::vector<void*>
typedef struct JArena JArena;

JBuffer*    jb_create(void);
void        jb_append(JBuffer*, const char*);
void        jb_appendf_a(JBuffer*, JArena*, const char* fmt, ...);
void        jb_print(JBuffer*);
char*       jb_str(JBuffer*);
char*       jb_str_a(JBuffer* jb, JArena* arena);
void        jb_free(JBuffer*);
void        jb_clear(JBuffer*);
bool        jb_eq(JBuffer*, const char*);
size_t      jb_len(JBuffer*);

void*    jmalloc(size_t);
void     jfree(void*);

JArena*  jarena_new(void);
void*    jarena_alloc(JArena*, size_t);
void*    jarena_zeroed(JArena*, size_t);
void     jarena_free(JArena*);
void     jarena_reset(JArena*);
char*    jarena_strdup(JArena*, char*);

// file utils
typedef struct CJBuffer CJBuffer;
size_t      jb_read_entire_file(const char*, CJBuffer*);
bool        jfile_write(const char*, const char*);
char*       jfile_stem(const char*, JArena*);
char*       jfile_ext(const char*, JArena*);
char*       jfile_dirname(const char*, JArena*);
char*       jfile_basename(const char*, JArena*);
bool        jfile_exists(const char*);
long        jfile_size(const char*);

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
