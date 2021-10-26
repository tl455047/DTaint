#include "dfsan_hook.h"
#include "dtaint.h"
#include "sanitizer_common/sanitizer_common.h"
#include "types.h" 
#include <cstring>
#include <stdint.h>

//extern struct d_tainted_map *__afl_d_tainted_map;

struct d_tainted_map *__afl_d_tainted_map;

void dfsan_hook_load_inst(dfsan_label_info *label_info, dfsan_label label, void* ptr, size_t size) {
  
 /* if (unlikely(!__afl_d_tainted_map) || !dfsan_check_label(label_info, label)) return;

  struct offset_node* node = (struct offset_node *)dfsan_union_t_get_offset(label);
  
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

  // Copy offset to shm.
  memcpy(__afl_d_tainted_map->tainted_data[k], node->tainted, node->num * sizeof(struct tainted));
  
  //fprintf(stderr, "Label %u loaded from memory %p size: %u\n", label, ptr, size);
  //dfsan_union_t_output_offset(label_info, label);
  dfsan_union_t_output_offset(label);*/
}

void dtaint_dump(dfsan_label_info *label_info) {
  if (unlikely(!__afl_d_tainted_map)) return;
  //__afl_d_tainted_map->tainted_bytes = label_info->tainted_bytes;
  //__afl_d_tainted_map->union_label = DFSAN_UNION_T_SIZE - label_info->union_label;
  //char str[64] = "Shm testing Hatsuyuki Sakura\n";
  //strncpy((char* )__afl_d_tainted_map, str, strlen(str));
  
}

void dtaint_set_shm(dfsan_label* addr, dfsan_label label) {
  if (unlikely(!__afl_d_tainted_map)) return;

  struct offset_node* node = (struct offset_node *)dfsan_union_t_get_offset(label);

  if (*addr == 0) 
    __afl_d_tainted_map->tainted_bytes++;
  
  __afl_d_tainted_map->tainted_data[__afl_d_tainted_map->num_of_update].pos = addr;
  __afl_d_tainted_map->tainted_data[__afl_d_tainted_map->num_of_update].len = node->len;

  __afl_d_tainted_map->num_of_update++;

}