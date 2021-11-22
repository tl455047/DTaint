#ifndef _DTAINT_H_
#define _DTAINT_H_
#define DTAINT_MAP_W 2 << 16
#define DTAINT_MAP_H 32


enum DFSanHookType {

    DFSH_UNKNOWN = 0,
    // __dfsan_hook1 (unsigned id, dfsan_label ptr_label);
    // ex. load inst.
    DFSH_HOOK1 = 1, 
    // __dfsan_hook2 (unsigned id, dfsan_label value_label, dfsan_label ptr_label);
    // ex. store inst.
    DFSH_HOOK2 = 2,
    // __dfsan_hook2_int128 (unsigned id, dfsan_label value_label, dfsan_label ptr_label);
    // deal with 16byte float point value
    DFSH_HOOK2_INT128 = 3,
    // __dfsan_hook3 (unsigned id, dfsan_label ptr_label, dfsan_label c_label, dfsan_label size_label);
    // ex. memset
    DFSH_HOOK3 = 4,
    // __dfsan_hook4 (unsigned id, dfsan_label dst_label, dfsan_label src_label, dfsan_label size_label);
    // ex. memcpy
    DFSH_HOOK4 = 5,
    // __dfsan_hook5 (unsigned id, dfsan_label size_label);
    // ex. malloc
    DFSH_HOOK5 = 6,
    // __dfsan_hook6 (unsigned id, dfsan_label ptr_label);
    // ex. free
    DFSH_HOOK6 = 7,
    // __dfsan_hook7 (unsigned id, dfsan_label ptr_label, dfsan_label size_label);
    // ex. realloc
    DFSH_HOOK7 = 8,
    // __dfsan_va_arg_hook1 (unsigned id, dfsan_label ptr_label, unsigned num_of_idx, ...);
    // ex. get_element_ptr
    DFSH_VARARG_HOOK1 = 9

};

struct dtaint_header {
  
  // instructions executed
  unsigned int hits;
  // unique id
  unsigned int id;
  //total tainted bytes
  unsigned int tainted_bytes;
  // num of node
  unsigned int num;
  //type
  unsigned int type;
  
} __attribute__((packed));

struct dtainted {

  unsigned int pos;
  unsigned int len;

} __attribute__((packed));

struct dtaint_map {
  
  struct dtaint_header headers[DTAINT_MAP_W];
  struct dtainted log[DTAINT_MAP_W][DTAINT_MAP_H]; 

};

#endif
