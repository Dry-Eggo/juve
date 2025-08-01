#include "juve_utils.h"
#include <sstream>
#include <string>
#include <cstdlib>
#include <cstring>
#include <stdarg.h>


struct JBuffer {
    std::stringstream buffer;
    JBuffer() {
    }
};

JBuffer* jb_create() {
    return new JBuffer();
}

void jb_print(JBuffer* jb) {
    if (!jb) return;
    printf("%s", jb->buffer.str().c_str());
}

void jb_append(JBuffer* jb, const char* str) {
    if (!jb) return;
    jb->buffer << str;
}

void jb_appendf_a(JBuffer* jb, JArena* arena, const char* fmt, ...) {

    if (!jb || !fmt) return;
    va_list args;
    va_start(args, fmt);
    size_t needed = vsnprintf(NULL, 0, fmt, args);

    va_list args_;
    va_start(args_, fmt);
    char* tmp = (char*)jarena_alloc(arena, needed + 1);
    vsnprintf(tmp, needed + 1, fmt, args_);
    va_end(args);
    va_end(args_);
    jb->buffer << tmp;
}

bool jb_eq(JBuffer* jb, const char* str) {
    std::string s = jb->buffer.str();
    return (s == str);
}

size_t jb_len(JBuffer* jb) {
    if (!jb) return -1;    
    return jb->buffer.str().size();
}

char* jb_str(JBuffer* jb) {
    if (!jb) return nullptr;
    std::string content = jb->buffer.str();
    char* res = static_cast<char*>(::operator new(content.size() + 1));
    std::memcpy(res, content.c_str(), content.size() + 1);
    return res;
}

char* jb_str_a(JBuffer* jb, JArena* arena) {
    if (!jb) return nullptr;
    
    std::string content = jb->buffer.str();
    return jarena_strdup(arena, (char*)content.data());
}

void jb_clear(JBuffer* jb) {
    if (!jb) return;

    jb->buffer.str("");
    jb->buffer.clear();
}

void jb_free(JBuffer* jb) {
    delete jb;
}
