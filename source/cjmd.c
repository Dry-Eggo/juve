#include <juve_utils.h>
#include <stdlib.h>
#include <stdio.h>

CJCmd jcmd_init(JArena* arena, JCmdOption opt) {
    return (CJCmd) {
        .buffer = cjb_create(arena),
        .opt = opt
    };
}

bool jcmd_run(CJCmd cmd) {
    const char* str = cjb_str(cmd.buffer);
    if (cmd.opt == JCMD_LOG) {
        printf("Running Command: %s\n", str);
    }
    return system(str);
}

void jcmd_append(CJCmd* cmd, const char* atom) {
    cjb_appendf(cmd->buffer, "%s ", atom);
}

bool jcmd_one_shot(CJCmd* cmd) {
    bool r = jcmd_run(*cmd);
    cjb_clear(cmd->buffer);
    return r;
}

void jcmd_reset(CJCmd* cmd) {
    cjb_clear(cmd->buffer);    
}
