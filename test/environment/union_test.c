#include <stdio.h>



struct hook1 {
    void* ptr;
    size_t size;
};

struct hook2 {
    void* ptr;
    size_t size;
    int c;
};

struct hook3 {
    void* src;
    void* dst;
    size_t size;
};

struct hook4 {
    void* ptr;
    int src_type;
    int rst_type;
};

struct hook5 {
    void* ptr;
    int src_type;
    int rst_type;
    size_t value;
};

union hook {
    struct hook1 __hook1;
    struct hook2 __hook2;
    struct hook3 __hook3;
    struct hook4 __hook4;
    struct hook5 __hook5;
};

int main(int argc, char* argv[]) {
    struct hook5 test_hook5;
    
    test_hook5.ptr = 0x10;
    test_hook5.rst_type = 8;
    test_hook5.src_type = 16;
    test_hook5.value = 9487;

    union hook test_hook;
    test_hook.__hook5 = test_hook5;

    fprintf(stderr, "test hook5: ptr: %p src_type: %u rst_type: %u value: %lld\n", 
        test_hook.__hook5.ptr, test_hook.__hook5.rst_type, test_hook.__hook5.src_type, test_hook.__hook5.value);

    fprintf(stderr, "test hook1: ptr: %p size: %lld", 
        test_hook.__hook1.ptr, test_hook.__hook1.size);
    return 0;
}