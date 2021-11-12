#ifndef _MEMLOG_H_
#define _MEMLOG_H_
#define MEMLOG_MAP_W 2 << 16
#define MEMLOG_MAP_H 32

enum HookType {
    HT_UNKNOWN = 0,
    // __memlog_hook1 (unsigned id, void* ptr, u8 src_type, u8 rst_type);
    // ex. load inst.
    HT_HOOK1 = 1, 
    // __memlog_hook2 (unsigned id, size_t value, int src_type, void* ptr, int rst_type);
    // ex. store inst.
    HT_HOOK2 = 2,
    // __memlog_hook3 (unsigned id, void* ptr, int c, size_t size);
    // ex. memset
    HT_HOOK3 = 3,
    // __memlog_hook4 (unsigned id, void* dst, void* src, size_t size);
    // ex. memcpy
    HT_HOOK4 = 4,
    // __memlog_hook5 (unsigned id, int type, size_t size);
    // ex. alloca
    HT_HOOK5 = 5,
    // __memlog_hook6 (unsigned id, size_t size);
    // ex. malloc
    HT_HOOK6 = 6,
    // __memlog_hook7 (unsigned id, void* ptr);
    // ex. free
    HT_HOOK7 = 7,
    // __memlog_hook8 (unsigned id, void* ptr, size_t size);
    // ex. realloc
    HT_HOOK8 = 8,
    // __memlog_vararg_hook1 (unsigned id, void* ptr, size_t size, size_t num_of_args, ...);
    // ex. get_element_ptr
    HT_VARARG_HOOK1 = 9,
    // __memlog_hook2_int128 (unsigned id, uint128_t value, int src_type, void* ptr, int rst_type);
    // deal with 16byte float point value
    HT_HOOK2_INT128 = 10
};

struct memlog_header {
  
  // instructions executed
  unsigned hits : 24;
  // unique id
  unsigned id : 24;
  // src shape 
  unsigned src_shape : 5;
  // result shape
  unsigned rst_shape : 5;
  //type
  unsigned type : 5;
  // padding
  unsigned reserved : 3;

} __attribute__((packed));


struct hook_operand {
  
  void* src;
  void* dst;
  unsigned long long size;
  unsigned long long value;
  unsigned long long value_128;

};

struct hook_va_arg_operand {
  
  unsigned offset;
  unsigned num;
  void* ptr;

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
  unsigned current_pos;
  struct hook_va_arg_idx va_arg_idx[MEMLOG_MAP_W]; // what is the proper size for this data section?

};

#endif