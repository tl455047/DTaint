/**
 * Runtime library for specific instruction hook and 
 * shared memory setup.
 */

//#include "../dfsan_rt/include/sanitizer/dfsan_interface.h"
#include <stdio.h>
#include <stdlib.h>

extern int __pipe_argc;
/*#include "../include/types.h"
u8 *taint_map;

void __taint_store_callback(dfsan_label Label) {
    fprintf(stderr, "Label %u stored to memory\n", Label);
}

void __taint_load_callback(dfsan_label Label) {
    fprintf(stderr, "Label %u loaded from memory\n", Label);
}

void __taint_mem_transfer_callback(dfsan_label *Start, size_t Len) {
    
    fprintf(stderr, "Label %u copied to memory\n", Start[0]);
}*/

void __attribute__((constructor)) argc_debug() {
    __pipe_argc = 9487;
    //asm volatile("mov  rdi, %0\n": "=r"(argc));
    fprintf(stderr, "__debug_main\nargc should be fucking %i Ye !!!!!!\n", __pipe_argc);
}



