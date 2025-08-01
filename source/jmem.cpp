#include <juve_utils.h>
#include <vector>
#include <cstring>

void* jmalloc(size_t n) {
    return ::operator new(n);
}

void jfree(void* ptr) {
    ::operator delete(ptr);
}

struct JArena {
    std::vector<void*> allocations;
    JArena() {}
};

JArena* jarena_new() {
    return new JArena();
}

void* jarena_alloc(JArena* arena, size_t n) {
    if (!arena) return nullptr;
    void* mem = jmalloc(n);
    if (mem) {
	arena->allocations.push_back(mem);
	return mem;
    }
    return nullptr;
}

void* jarena_zeroed(JArena* arena, size_t n) {
    if (!arena) return nullptr;
    void* mem = jmalloc(n);
    if (mem) {
	memset(mem, 0, n);
	arena->allocations.push_back(mem);
	return mem;
    }
    return nullptr;    
}

char* jarena_strdup(JArena* arena, char* str) {
    if (!arena || !str) return nullptr;
    size_t str_len = strlen(str) + 1;
    char* new_str = (char*)jarena_alloc(arena, str_len);
    memcpy(new_str, str, str_len);
    return new_str;
}

void jarena_reset(JArena* arena) {
    if (!arena) return;
    for (void* mem: arena->allocations) jfree(mem);
    arena->allocations.clear();
}

void jarena_free(JArena* arena) {
    if (!arena) return;
    for (void* mem: arena->allocations) jfree(mem);
    delete arena;
}
