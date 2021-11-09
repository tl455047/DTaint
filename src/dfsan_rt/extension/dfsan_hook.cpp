#include "dfsan_hook.h"
#include "dtaint.h"
#include "sanitizer_common/sanitizer_common.h"
#include "types.h" 
#include <cstring>
#include <stdint.h>
#include <stdarg.h>
/**
 * Call hook.
 * __memlog_hook1 (void* ptr, u8 src_type, u8 rst_type);
 * ex. load
 * __memlog_hook2 (size_t value, int src_type, void* ptr, int rst_type);
 * ex. store
 * __memlog_hook3 (void* ptr, int c, size_t size);
 * ex. memset
 * __memlog_hook4 (void* dst, void* src, size_t size);
 * ex. memcpy
 * __memlog_hook5 (int type, size_t size);
 * ex. alloca
 * 
 * __memlog_vararg_hook1 (void* ptr, size_t size, size_t num_of_args, ...);
 * ex. get_element_ptr
 * 
 * 
 */
extern struct dfsan_label_info __dfsan_label_info;
struct d_tainted_map *__afl_d_tainted_map;

/**
 * load inst.
 */
extern "C" SANITIZER_INTERFACE_ATTRIBUTE
void __memlog_hook1(void* ptr, int src_type, int rst_type) {
  fprintf(stderr, "__memlog_hook1: ptr: %p src_type: %d rst type:\
  %d\n", ptr, src_type, rst_type);
}

/**
 * store inst.
 */
extern "C" SANITIZER_INTERFACE_ATTRIBUTE
void __memlog_hook2(size_t value, int src_type, void* ptr, int rst_type) {
  fprintf(stderr, "__memlog_hook2: value: %x src_type: %d\
  ptr: %p rst_type: %d\n", value, src_type, ptr, rst_type);
}

/**
 * memset
 */
extern "C" SANITIZER_INTERFACE_ATTRIBUTE
void __memlog_hook3(void* ptr, int c, size_t size) {
  fprintf(stderr, "__memlog_hook3: ptr: %p c: %d size: %lld\n", ptr, c, size);
}

/**
 * memcpy
 */
extern "C" SANITIZER_INTERFACE_ATTRIBUTE
void __memlog_hook4(void* dst, void* src, size_t size) {
  fprintf(stderr, "__memlog_hook4: dst: %p src: %p size: %lld\n", dst, src, size);
}

/**
 * alloca inst.
 */
extern "C" SANITIZER_INTERFACE_ATTRIBUTE
void __memlog_hook5(int type, size_t size) {
  fprintf(stderr, "__memlog_hook5: type: %d size: %lld\n", type, size);
}

/**
 * get_element_ptr inst.
 */
extern "C" SANITIZER_INTERFACE_ATTRIBUTE
void __memlog_vararg_hook1(void* ptr, int src_type, int rst_type, int num_of_idx, ...) {
  //deal with vararg
  va_list args;
  va_start(args, num_of_idx);

  fprintf(stderr, "__memlog_vararg_hook1: ptr: %p src_type: %d\
  rst_type: %d offset: %d ", ptr, src_type, rst_type, num_of_idx);
  
  int size;
  for(int j = 0; j < num_of_idx; j++) {
    size = va_arg(args, int);
    switch(size) {
      case 1:
        fprintf(stderr, "%d ", va_arg(args, char));
        break;
      case 2:
        fprintf(stderr, "%d ", va_arg(args, short));
        break;
      case 4:
        fprintf(stderr, "%d ", va_arg(args, int));
        break;
      case 8:
        fprintf(stderr, "%lld ", va_arg(args, long long));
        break;
      default:
      // what fucking happened ?
        break;
    }
  }fprintf(stderr, "\n");

  va_end(args);
}

extern "C" SANITIZER_INTERFACE_ATTRIBUTE
void __hook_debug(size_t num1, size_t num2) {
    
  //if (unlikely(!__afl_d_tainted_map)) return;
  fprintf(stderr, "__hook_debug: num1: %d num2: %d\n", num1, num2);
    
}

/**
 * load inst.
 */
extern "C" SANITIZER_INTERFACE_ATTRIBUTE
void __dtaint_hook1(void* ptr, int src_type, int rst_type, dfsan_label ptr_label) {
  fprintf(stderr, "__memlog_hook1: ptr: %p src_type: %d rst type:\
  %d\n", ptr, src_type, rst_type);
}

/**
 * store inst.
 */
extern "C" SANITIZER_INTERFACE_ATTRIBUTE
void __dtaint_hook2(size_t value, int src_type, void* ptr, int rst_type, 
dfsan_label value_label, dfsan_label ptr_label) {
  fprintf(stderr, "__memlog_hook2: value: %x src_type: %d\
  ptr: %p rst_type: %d\n", value, src_type, ptr, rst_type);
}

/**
 * memset
 */
extern "C" SANITIZER_INTERFACE_ATTRIBUTE
void __dtaint_hook3(void* ptr, int c, size_t size, dfsan_label ptr_label,
dfsan_label c_label, dfsan_label size_label) {
  fprintf(stderr, "__memlog_hook3: ptr: %p c: %d size: %lld\n", ptr, c, size);
}

/**
 * memcpy
 */
extern "C" SANITIZER_INTERFACE_ATTRIBUTE
void __dtaint_hook4(void* dst, void* src, size_t size, dfsan_label dst_label, 
dfsan_label src_label, dfsan_label size_label) {
  fprintf(stderr, "__memlog_hook4: dst: %p src: %p size: %lld\n", dst, src, size);
}

/**
 * alloca
 */
extern "C" SANITIZER_INTERFACE_ATTRIBUTE
void __dtaint_hook5(int type, size_t size, dfsan_label size_label) {
  fprintf(stderr, "__memlog_hook5: type: %d size: %lld\n", type, size);
}

/**
 * get_element_ptr inst.
 */
extern "C" SANITIZER_INTERFACE_ATTRIBUTE
void __dtaint_vararg_hook1(void* ptr, int src_type, int rst_type, 
dfsan_label ptr_label, int num_of_idx, ...) {
  //deal with vararg
  va_list args;
  va_start(args, num_of_idx);

  fprintf(stderr, "__memlog_vararg_hook1: ptr: %p src_type: %d\
  rst_type: %d offset: %d ", ptr, src_type, rst_type, num_of_idx);
  
  int size;
  for(int j = 0; j < num_of_idx; j++) {
    size = va_arg(args, int);
    switch(size) {
      case 1:
        fprintf(stderr, "%d ", va_arg(args, char));
        break;
      case 2:
        fprintf(stderr, "%d ", va_arg(args, short));
        break;
      case 4:
        fprintf(stderr, "%d ", va_arg(args, int));
        break;
      case 8:
        fprintf(stderr, "%lld ", va_arg(args, long long));
        break;
      default:
      // what fucking happened ?
        break;
    }
  }fprintf(stderr, "\n");

  va_end(args);
}