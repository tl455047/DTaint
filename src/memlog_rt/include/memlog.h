#ifndef _MEMLOG_H_
#define _MEMLOG_H_
#define MEMLOG_MAP_W 2 << 16
#define MEMLOG_MAP_H 32
#define MEMLOG_MAXiMUM_IDX_NUM 8

enum HookType {

    HT_UNKNOWN = 0,
    // __memlog_hook1 (unsigned id, void* ptr, unsigned src_type, unsigned rst_type);
    // ex. load inst.
    HT_HOOK1 = 1, 
    // __memlog_hook2 (unsigned id, size_t value, unsigned src_type, void* ptr, unsigned rst_type);
    // ex. store inst.
    HT_HOOK2 = 2,
    // __memlog_hook2_int128 (unsigned id, uint128_t value, unsigned src_type, void* ptr, unsigned rst_type);
    // deal with 16byte float point value
    HT_HOOK2_INT128 = 3,
    // __memlog_hook3 (unsigned id, void* ptr, int c, size_t size);
    // ex. memset
    HT_HOOK3 = 4,
    // __memlog_hook4 (unsigned id, void* dst, void* src, size_t size);
    // ex. memcpy
    HT_HOOK4 = 5,
    // __memlog_hook5 (unsigned id, size_t size);
    // ex. malloc
    HT_HOOK5 = 6,
    // __memlog_hook6 (unsigned id, void* ptr);
    // ex. free
    HT_HOOK6 = 7,
    // __memlog_hook7 (unsigned id, void* ptr, size_t size);
    // ex. realloc
    HT_HOOK7 = 8,
    // __memlog_va_arg_hook1 (unsigned id, void* ptr, unsigned src_type, 
    // unsigned rst_type, unsigned num_of_idx, ...);
    // ex. get_element_ptr
    HT_VARARG_HOOK1 = 9

};

struct memlog_header {
  
  // instructions executed
  unsigned hits;
  // unique id
  unsigned id;
  // src shape 
  unsigned src_shape;
  // result shape
  unsigned rst_shape;
  //type
  unsigned type;
  
} __attribute__((packed));


struct hook_operand {
  
  void* src;
  void* dst;
  unsigned long long size;
  unsigned long long value;
  unsigned long long value_128;

};

struct hook_va_arg_operand {
  
  unsigned num;
  void* ptr;
  unsigned idx[MEMLOG_MAXiMUM_IDX_NUM];

} __attribute__((packed));

struct hook_va_arg_idx {
  
  unsigned type;
  unsigned long long idx;

} __attribute__((packed));

union hook_operands {

  struct hook_operand __hook_op;
  struct hook_va_arg_operand __hook_va_arg;

};

struct memlog_map {
  
  struct memlog_header headers[MEMLOG_MAP_W];
  union hook_operands log[MEMLOG_MAP_W][MEMLOG_MAP_H]; 

};

#endif