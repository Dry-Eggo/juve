#pragma once
#include "juve_utils.h"

typedef struct {
    int  indent;
    JArena* arena;
} jtab_tracker_t;


jtab_tracker_t jtab_new(JArena* arena);

void jtab_add_level(jtab_tracker_t* tracker);
void jtab_sub_level(jtab_tracker_t* tracker);

const char* jtab_to_str(jtab_tracker_t* tracker);
