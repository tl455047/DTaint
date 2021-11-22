#ifndef _DTAINT_H_
#define _DTAINT_H_

#define DTAINT_MAP_W 2 << 16
#define DTAINT_MAP_H 32
#define DTAINT_MAXiMUM_IDX_NUM 8

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

struct dtainted {

  unsigned int pos;
  unsigned int len;

} __attribute__((packed));


struct dtaint_offset_node {
  
  //total tainted bytes
  unsigned int len;
  // num of node
  unsigned int num;

} __attribute__((packed));

struct va_arg_label_t {

  unsigned int num;
  unsigned int ptr_label;
  unsigned int idx_label[DTAINT_MAXiMUM_IDX_NUM];

} __attribute__((packed));

struct label_t {

  unsigned int   src_label;
  unsigned int   dst_label;
  unsigned int  size_label;
  unsigned int value_label;

};

union dtaint_label_t {

  struct va_arg_label_t __va_arg_label_t;
  struct label_t __label_t;

};

struct dtaint_header {
  
  // instructions executed
  unsigned int hits;
  // unique id
  unsigned int id;
  //type
  unsigned int type;
  //label array
  
} __attribute__((packed));

struct dtaint_map {
  
  struct dtaint_header headers[DTAINT_MAP_W];
  union dtaint_label_t log[DTAINT_MAP_W][DTAINT_MAP_H]; 
  struct dtaint_offset_node label_info[DTAINT_MAP_W];
  struct dtainted offset_t[DTAINT_MAP_W][DTAINT_MAP_H]; 

};

#endif
