#include "dfsan_hook.h"
#include "dtaint.h"
#include "sanitizer_common/sanitizer_common.h"
#include "types.h" 
#include <cstdio>
#include <cstring>
#include <stdint.h>

struct d_tainted_map *__afl_d_tainted_map;

void offset_cpy(struct d_tainted *dst,  struct tainted* t) {
  u32 i = 0;
  while(t != NULL && i < TAINTED_MAP_H) {
    dst[i].pos = t->pos;
    dst[i].len = t->len;
    t = t->next;
    i += 1;
  } 
}

void dfsan_hook_load_inst(dfsan_label_info *label_info, dfsan_label label, void* ptr, size_t size) {
  
  if (unlikely(!__afl_d_tainted_map) || !dfsan_check_label(label_info, label)) return;

  struct offset_node* node = (struct offset_node *)dfsan_union_t_get_offset(label_info, label);
  
  uintptr_t k = (uintptr_t)__builtin_return_address(0);
  k = (k >> 4) ^ (k << 8);
  k &= TAINTED_MAP_W - 1;
 
  // Set shm offset_node.
  if(__afl_d_tainted_map->header[k].num < node->num || __afl_d_tainted_map->header[k].len < node->len) {
    __afl_d_tainted_map->header[k].num = node->num;
    __afl_d_tainted_map->header[k].len = node->len;
  }
  else
    return;
  
  // set instruction header 
  __afl_d_tainted_map->header[k].shape = size;
  __afl_d_tainted_map->header[k].ptr = ptr;

  if(size == 1)
    __afl_d_tainted_map->header[k].value = *(char *)ptr;
  else if(size == 2)
    __afl_d_tainted_map->header[k].value = *(unsigned short *)ptr;
  else if(size == 4)
    __afl_d_tainted_map->header[k].value = *(unsigned int *)ptr;
  else 
    __afl_d_tainted_map->header[k].value = *(unsigned long long *)ptr;
  // Copy offset to shm.
  offset_cpy(__afl_d_tainted_map->tainted_data[k], node->tainted);
  
  //fprintf(stderr, "Label %u loaded from memory %p size: %u\n", label, ptr, size);
  //dfsan_union_t_output_offset(label_info, label);
  
}

