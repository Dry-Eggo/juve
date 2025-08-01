#include "misc.h"

jtab_tracker_t jtab_new(JArena* arena) {
    return (jtab_tracker_t) { 0, arena };
}

void jtab_add_level(jtab_tracker_t* tracker) { tracker->indent++ ;}
void jtab_sub_level(jtab_tracker_t* tracker) { tracker->indent -= tracker->indent <= 0 ? 0 : 1; }

const char* jtab_to_str(jtab_tracker_t* tracker) {
    if (tracker->indent <= 0) return "";

    size_t indent_level = (tracker->indent * 4);
    char* repr = jarena_alloc(tracker->arena, indent_level + 1);
    size_t len = 0;
    for (; len < indent_level; ++len) {
        repr[len] = ' ';
    }
    repr[len] = '\0';
    return repr;
}
