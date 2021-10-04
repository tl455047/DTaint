#include <stdio.h>
#include <sys/mman.h>

/**
 * Test for internel_mmap in DFSan, cause SegFault.
 */
int main() {
    
    void* addr = mmap((void *)0x10000, 0x200200000000 - 0x10000, 0x3, 0x4032, 0xffffffffffffffff, 0x0);
    printf("ok. address is %p\n", addr);
    return 0;
}