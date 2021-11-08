#include "dfsan_hook.h"
#include "dtaint.h"
#include "sanitizer_common/sanitizer_common.h"
#include "types.h" 
#include <cstring>
#include <stdint.h>

/**
 * Call hook.
 * __memlog_hook1 (size_t size);
 * ex. malloc
 * __memlog_hook2 (void* ptr);
 * ex. free
 * __memlog_hook3 (void* ptr, size_t size);
 * ex. read
 * __memlog_hook4 (void* dst, void* src, size_t size);
 * ex. strncpy
 * 
 * instruction hook.
 * __memlog_hook (void* ptr, size_t size);
 * ex. load
 * __memlog_hook5 (void* ptr, size_t offset, size_t size);
 * ex. get_element_ptr 
 * __memlog_hook6 (void* ptr, size_t value, size_t size);
 * ex. store
 * __memlog_hook7 (size_t len, size_t size); 
 * // here size means allocated size, len means each element size
 * __memlog_hook8 (void* ptr, size_t offset); 
 * 
 * __tainted_hook (dfsan_label label, size_t size);
 * __tainted_hook (dfsan_label label, void* ptr);
 * __tainted_hook (dfsan_label label, void* ptr, size_t size);
 * __tainted_hook (dfsan_label label, void* dst, void* src, size_t size);
 * 
 */
extern struct dfsan_label_info __dfsan_label_info;
struct d_tainted_map *__afl_d_tainted_map;

extern "C" SANITIZER_INTERFACE_ATTRIBUTE
void __memlog_hook1(size_t size) {

}

extern "C" SANITIZER_INTERFACE_ATTRIBUTE
void __memlog_hook2(void* ptr) {

}
extern "C" SANITIZER_INTERFACE_ATTRIBUTE
void __memlog_hook3(void* ptr, size_t size) {
  fprintf(stderr, "__memlog_hook3: ptr: %p size: %lld\n", ptr, size);
}

extern "C" SANITIZER_INTERFACE_ATTRIBUTE
void __memlog_hook4(void* dst, void* src, size_t size) {
  fprintf(stderr, "__memlog_hook4: dst: %p src: %p size: %lld\n", dst, src, size);
}

extern "C" SANITIZER_INTERFACE_ATTRIBUTE
void __memlog_hook5(void* ptr, size_t offset, size_t size) {
  fprintf(stderr, "__memlog_hook5: ptr: %p offset: %lld size: %lld\n", ptr, offset, size);
}

extern "C" SANITIZER_INTERFACE_ATTRIBUTE
void __memlog_hook6(void* ptr, size_t value, size_t size) {
  fprintf(stderr, "__memlog_hook6: ptr: %p size: %u value: %x\n", ptr, size, value);
}

extern "C" SANITIZER_INTERFACE_ATTRIBUTE
void __memlog_hook7(size_t len, size_t size) {
  fprintf(stderr, "__memlog_hook7: len: %lld size: %lld\n", len, size);
}

extern "C" SANITIZER_INTERFACE_ATTRIBUTE
void __memlog_hook8(void* ptr, size_t offset) {
   fprintf(stderr, "__memlog_hook8: ptr: %p offset: %lld\n", ptr, offset);
}

extern "C" SANITIZER_INTERFACE_ATTRIBUTE
void __hook_debug(void* ptr, size_t size) {
    
  //if (unlikely(!__afl_d_tainted_map)) return;
  fprintf(stderr, "__hook_debug: label: %p size: %p\n", ptr, size);
    
}

extern "C" SANITIZER_INTERFACE_ATTRIBUTE
void __hook_malloc(unsigned int label, size_t size) {
  
  if (unlikely(!__afl_d_tainted_map) || !label 
  || !dfsan_check_label(&__dfsan_label_info, label)) return;

  uintptr_t k = (uintptr_t)__builtin_return_address(0);
  k = (k >> 4) ^ (k << 8);
  k &= TAINTED_MAP_W - 1;

  // check if the label is same with previous label. 
  if (__afl_d_tainted_map->headers[k].label == label) return;

  u32 hits;
  //increase hit count
  hits = __afl_d_tainted_map->headers[k].hits++;
  __afl_d_tainted_map->headers[k].shape0 = 0;
  // set attr

  hits &= TAINTED_MAP_H - 1;
  // store operands
  //__afl_d_tainted_map->log[k][hits].v0 = size;
                                                                                                                                                                                             
  // get label offset
  struct offset_node * node = (struct offset_node* )dfsan_union_t_get_offset(label);

  // set bytes, num, label
  __afl_d_tainted_map->headers[k].bytes = node->len;
  __afl_d_tainted_map->headers[k].num = node->num;
  __afl_d_tainted_map->headers[k].label = label;

  // set offset
  memcpy(__afl_d_tainted_map->offset[k], node->tainted, node->num * sizeof(struct tainted));

}

extern "C" SANITIZER_INTERFACE_ATTRIBUTE
void __hook_free(unsigned int label, void* ptr) {

  if (unlikely(!__afl_d_tainted_map) || !label) return;

}

extern "C" SANITIZER_INTERFACE_ATTRIBUTE
void __hook_load(unsigned int label, void* ptr) {
    
  //if (unlikely(!__afl_d_tainted_map)) return;
  fprintf(stderr, "__hook_load: label: %u, ptr: %p\n", label, ptr);
    
}

extern "C" SANITIZER_INTERFACE_ATTRIBUTE
void __hook_get_element_ptr(unsigned int label, void* ptr) {
    
  //if (unlikely(!__afl_d_tainted_map)) return;
  fprintf(stderr, "__hook_get_element_ptr: label: %u, ptr: %p\n", label, ptr);
    
}
/*void dtaint_dump(dfsan_label_info *label_info) {
  if (unlikely(!__afl_d_tainted_map)) return;
  
  __afl_d_tainted_map->header.union_label = DFSAN_UNION_T_SIZE - label_info->union_label;
  __afl_d_tainted_map->header.input_label = label_info->input_label;
  __afl_d_tainted_map->header.last_label = label_info->last_label;
  //__afl_d_tainted_map->union_label = DFSAN_UNION_T_SIZE - label_info->union_label;
  //char str[64] = "Shm testing Hatsuyuki Sakura\n";
  //strncpy((char* )__afl_d_tainted_map, str, strlen(str));
  
}

void dtaint_set_shm(dfsan_label* addr, dfsan_label label) {
  if (unlikely(!__afl_d_tainted_map) || __afl_d_tainted_map->header.num_of_update >= TAINTED_MAP_W) return;

  struct offset_node* node = (struct offset_node *)dfsan_union_t_get_offset(label);

  if (*addr == 0) 
    __afl_d_tainted_map->header.tainted_bytes++;
  
  __afl_d_tainted_map->tainted_data[__afl_d_tainted_map->header.num_of_update].pos = addr;
  __afl_d_tainted_map->tainted_data[__afl_d_tainted_map->header.num_of_update].len = node->len;

  __afl_d_tainted_map->header.num_of_update++;

}*/