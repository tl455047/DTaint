#include <stdio.h>


static void preinit_debug() {
    printf("preinit\n");
}

__attribute__((section(".preinit_array"), used))
static void (*preinit_debug_ptr)(int, char **, char **) = preinit_debug;
__attribute((constructor)) void init_debug() {
    printf("__libc_csu_init\n");
}

int main(int argc, char** argv[]) {
    
    return 0;
}
