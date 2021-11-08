/**
 * Runtime library for specific instruction hook and 
 * shared memory setup.
 */

//#include "../dfsan_rt/include/sanitizer/dfsan_interface.h"
#include <stdio.h>
#include <stdlib.h>

/*void __hook_debug(unsigned int label, size_t size) {
    fprintf(stderr, "__hook_debug:  label: %u, size: %lld\n", label, size);
}

void __hook_malloc(unsigned int label, size_t size) {
    if(label)
        fprintf(stderr, "__hook_malloc: label: %u, size: %lld\n", label, size);
}

void __hook_free(unsigned int label, void* ptr) {
    if(label)
    fprintf(stderr, "__hook_free: label: %u, size: %p\n", label, ptr);
}*/