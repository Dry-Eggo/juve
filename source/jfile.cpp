#include <juve_utils.h>
#include <fstream>
#include <string>
#include <sstream>
#include <cstdlib>
#include <cstring>
#include <filesystem>

namespace fs = std::filesystem;

size_t jb_read_entire_file(const char* path, CJBuffer* jb) {
    std::fstream file(path);

    if (!file.is_open()) return -1;
    
    std::stringstream ss;
    ss << file.rdbuf();
    std::string content = ss.str();
    cjb_append(jb, content.c_str());
    return ss.str().size();
}

bool jfile_write(const char* path, const char* content) {
    if (!path || !content) return false;

    std::ofstream file(path, std::ios::binary);
    if (!file) return false;
    file << content;
    return true;
}

char*  jfile_stem(const char* path, JArena* arena) {
    if (!path || !arena) return nullptr;
    
    fs::path fpath(path);
    return jarena_strdup(arena, (char*)fpath.stem().string().c_str());
}

char*  jfile_ext(const char* path, JArena* arena) {
    if (!path || !arena) return nullptr;
    fs::path fpath(path);
    std::string ext = fpath.extension().string();
    if (!ext.empty() && ext[0] == '.') ext = ext.substr(1); // drop '.'
    return jarena_strdup(arena, (char*)ext.c_str());
}

char* jfile_dirname(const char* path, JArena* arena) {
    if (!path || !arena) return nullptr;
    fs::path fpath(path);
    return jarena_strdup(arena, (char*)fpath.parent_path().string().c_str());
}

char* jfile_basename(const char* path, JArena* arena) {
    if (!path || !arena) return nullptr;
    fs::path fpath(path);
    return jarena_strdup(arena, (char*)fpath.filename().string().c_str());
}

bool jfile_exists(const char* path) {
    if (!path) return false;
    return fs::exists(path);
}

long jfile_size(const char* path) {
    if (!path) return -1;
    std::error_code ec;
    auto size = fs::file_size(path, ec);
    if (ec) return -1;
    return static_cast<long>(size);
}
