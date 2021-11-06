#include "dfsan_hook.h"
#include "dtaint.h"
#include "sanitizer_common/sanitizer_common.h"
#include "types.h" 
#include <cstring>
#include <stdint.h>

//extern struct d_tainted_map *__afl_d_tainted_map;

struct d_tainted_map *__afl_d_tainted_map;


void __dfsan_hook_malloc() {
  fprintf(stderr, "hook malloc\n");
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