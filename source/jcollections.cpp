#include <juve_utils.h>
#include <vector>
#include <unordered_map>
#include <string>
#include <sstream>

struct JVec {
    std::vector<void*> data;
    JVec() {}
};

JVec* jvec_new() {
    return new JVec();
}

JVec* jvec_lines(const char* source, JArena* arena) {
    JVec* vec = jvec_new();
    std::stringstream ss;
    ss << source;
    std::string line;
    while (std::getline(ss, line)) {
	    jvec_push(vec, (void*)jarena_strdup(arena, (char*)line.c_str()));
    }
    return vec;
}

void jvec_push(JVec* vec, void* ptr) {
    if (!vec || !ptr) return;
    vec->data.push_back(ptr);
}

size_t jvec_len(JVec* vec) {
    if (!vec) return -1;
    return vec->data.size();
}

void*  jvec_at(JVec* vec, size_t n) {
    if (!vec) return nullptr;
    if (n >= vec->data.size()) return nullptr;
    return vec->data.at(n);
}

void* jvec_back(JVec* vec) {
    if (!vec) return nullptr;
    return vec->data.back();
}

void  jvec_free(JVec* vec) {
    if(!vec) return;
    delete vec;
}
