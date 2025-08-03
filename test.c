#define JUVE_IMPLEMENTATION
#include "juve_utils.h"

int main(void) {
    JArena arena = jarena_new();
    printf("%s\n", jfile_ext("/home/dry/programming/c/cog/build.py", &arena));
}
