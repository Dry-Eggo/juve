#include <juve_utils.h>
#include <string.h>
#include <stdint.h>

#define CJVEC_INIT 255

struct CJVec {
    void** data;
    size_t cap;
    size_t len;
    JArena* arena;
};

CJVec* cjvec_new(JArena* arena) {
    CJVec* vec = (CJVec*)jarena_alloc(arena, sizeof(CJVec));
    if (!vec) return NULL;
    
    vec->cap = CJVEC_INIT;
    vec->len = 0;
    vec->arena = arena;

    vec->data = (void**)jarena_alloc(arena, sizeof(void*)*vec->cap);
    if (!vec->data) return NULL;
    
    return vec;
}

CJVec* cjvec_lines(void);

void cjvec_push(CJVec* vec, void* data) {

    if (vec->len >= vec->cap) {
        size_t newsz = vec->cap*2;
        void** new_data = (void**)jarena_alloc(vec->arena, newsz);       
        
        memcpy(new_data, vec->data, vec->len);
        vec->data = new_data;
        vec->cap = newsz;
    }
    vec->data[vec->len++] = data;
}

size_t cjvec_len(CJVec* vec) {
    if (!vec) return -1;
    return vec->len;
}

void*  cjvec_at(CJVec* vec, size_t n) {
    if (!vec) return NULL;
    return vec->data[n];
}

void* cjvec_back(CJVec* vec) {
    if (!vec) return NULL;
    return vec->data[vec->len-1];    
}

void  cjvec_free(CJVec* vec) {
    (void) vec;
    // don't free anything.
    // structure is allocated on the arena
}

typedef struct Bucket_ {
    const char* key;
    void* value;
    struct Bucket_* next;
} Bucket;

#define MAX_BUCKETS 35

struct CJMap {
    Bucket* buckets[MAX_BUCKETS];
    JArena* arena;
};

uint32_t chash(const char* str) {    
    uint32_t hash = 5381;
    int c;

    while ((c = *str++))
        hash = ((hash << 5) + hash) + c; /* hash * 33 + c */

    return hash;
}

CJMap* cjmap_create(JArena* arena) {
    CJMap* map = jarena_zeroed(arena, sizeof(CJMap));
    memset(map->buckets, 0, sizeof(map->buckets));
    map->arena = arena;
    return map;
}

bool cjmap_put(CJMap* map, const char* key, void* value) {
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

bool cjmap_has(CJMap* map, const char* key) {
    uint32_t index = chash(key) % MAX_BUCKETS;
    Bucket*  current = map->buckets[index];
    while (current) {
        if (strcmp(current->key, key) == 0) return true;
        current = current->next;
    }
    return false;
}

bool cjmap_remove(CJMap* map, const char* key) {
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

void* cjmap_get(CJMap* map, const char* key) {
    uint32_t index = chash(key) % MAX_BUCKETS;
    Bucket* current = map->buckets[index];
    while (current) {
        if (strcmp(current->key, key) == 0) return current->value;
        current = current->next;
    }
    return NULL;
}
