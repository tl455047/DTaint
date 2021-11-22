#include "dtaint.h"
#include "types.h" 
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include "dfsan_union_t.h"
typedef unsigned __int128 uint128_t;

extern struct dfsan_label_info __dfsan_label_info;
extern struct dtaint_map *__afl_dtaint_map;

/**
 * Call hook.
 * __dfsan_hook1 (unsigned id, dfsan_label ptr_label);
 * ex. load inst.
 * __dfsan_hook2 (unsigned id, dfsan_label value_label, dfsan_label ptr_label);
 * ex. store inst.
 * __dfsan_hook2_int128 (unsigned id, dfsan_label value_label, dfsan_label ptr_label);
 * deal with 16 byte value
 * __dfsan_hook3 (unsigned id, dfsan_label ptr_label, dfsan_label c_label, dfsan_label size_label);
 * ex. memset
 * __dfsan_hook4 (unsigned id, dfsan_label dst_label, dfsan_label src_label, dfsan_label size_label);
 * ex. memcpy
 * __dfsan_hook5 (unsigned id, dfsan_label size_label);
 * ex. malloc
 * __dfsan_hook6 (unsigned id, dfsan_label ptr_label);
 * ex. free
 * __dfsan_hook7 (unsigned id, dfsan_label ptr_label, dfsan_label size_label);
 * ex. realloc
 * __dfsan_va_arg_hook1 (unsigned id, dfsan_label ptr_label, unsigned num_of_idx, ...);
 * ex. get_element_ptr inst.
 * 
 */

#ifdef DTAINT_DEBUG

extern "C" __attribute__((visibility("default")))
void __dfsan_hook_debug_output() {
  
  for (int i = 0; i < DTAINT_MAP_W; i++) {
    
    if (!__afl_dtaint_map->headers[i].hits) continue;

    switch (__afl_dtaint_map->headers[i].type) {
      case DFSH_VARARG_HOOK1: {
          
        fprintf(stderr, "header: id: %u hits: %u type: %u\n", 
          __afl_dtaint_map->headers[i].id, 
          __afl_dtaint_map->headers[i].hits,
          __afl_dtaint_map->headers[i].type);
        fprintf(stderr, "hook va arg log: num: %u ptr_label: %u\n",
          __afl_dtaint_map->log[i][0].__va_arg_label_t.num,
          __afl_dtaint_map->log[i][0].__va_arg_label_t.ptr_label);

        dfsan_label *p = __afl_dtaint_map->log[i][0].__va_arg_label_t.idx_label;
        for (int j = 0; j < __afl_dtaint_map->log[i][0].__va_arg_label_t.num; j++) {
          fprintf(stderr, "%u ", p[j]);
        }fprintf(stderr, "\n");
        break;
      }
      default:
        fprintf(stderr, "hook log: dst_label: %u src_label: %u value_label: %u size_label: %u\n",
          __afl_dtaint_map->log[i][0].__label_t.dst_label,
          __afl_dtaint_map->log[i][0].__label_t.src_label,
          __afl_dtaint_map->log[i][0].__label_t.value_label,
          __afl_dtaint_map->log[i][0].__label_t.size_label);
        break;
    }

  }

}


extern "C" __attribute__((visibility("default")))
void __dfsan_hook_debug_dump_offset_t() {

  for (int i = 0; i < DTAINT_MAP_W; i++) {

    if (__afl_dtaint_map->label_info[i].num == 0) continue;

    fprintf(stderr, "label info: label: %u len: %u num: %u\n", i, 
      __afl_dtaint_map->label_info[i].len,
      __afl_dtaint_map->label_info[i].num);

    fprintf(stderr, "offset_t: ");
    for (int j = 0; j < __afl_dtaint_map->label_info[i].num; j++) {
      fprintf(stderr, "pos: %u len: %u ", __afl_dtaint_map->offset_t[i][j].pos,
        __afl_dtaint_map->offset_t[i][j].len);
    } fprintf(stderr, "\n");

  }
}

extern "C" __attribute__((constructor)) 
void __dfsan_hook_debug_init() {
      
  fprintf(stderr, "__dfsan_debug_init\n");

  if(!__afl_dtaint_map) {
      __afl_dtaint_map = (struct dtaint_map *)malloc(sizeof(struct dtaint_map));
    memset(__afl_dtaint_map, 0, sizeof(struct dtaint_map));
  }

}

extern "C" __attribute__((destructor)) 
void __dfsan_hook_debug_fini() {

  fprintf(stderr, "__dfsan_hook_debug_fini\n");
  if (__afl_dtaint_map) {
  
    __dfsan_hook_debug_output();
    __dfsan_hook_debug_dump_offset_t();

  }
  if(__afl_dtaint_map) 
    free(__afl_dtaint_map);

}

extern "C" __attribute__((visibility("default")))
void __dfsan_hook_debug() {
  
  fprintf(stderr, "__dfsan_hook_debug\n");


}

#endif
/**
 * Copy offset to shm.
 */
void offset_cpy(dfsan_label label) {

  struct offset_node* node = (struct offset_node *)dfsan_union_t_get_offset(label);

  __afl_dtaint_map->label_info[label].num = node->num;
  __afl_dtaint_map->label_info[label].len = node->len;
  
  //move offset
  u32 num;
  if (node->num > DTAINT_MAP_H)
    num = DTAINT_MAP_H;
  else 
    num = node->num;
  
  for (int j = 0; j < num; j++) {

    __afl_dtaint_map->offset_t[label][j].pos = node->tainted[j].pos;
    __afl_dtaint_map->offset_t[label][j].len = node->tainted[j].len;
    
  }

}

/**
 * ex. load inst.
 */
extern "C" __attribute__((visibility("default")))
void __dfsan_hook1 (unsigned id, dfsan_label ptr_label) {
  #ifdef DTAINT_DEBUG

  #endif 
  // not handling
}
/**
 * ex. load inst.
 */
extern "C" __attribute__((visibility("default")))
void __dfsan_hook2 (unsigned id, dfsan_label value_label, dfsan_label ptr_label) {
  #ifdef DTAINT_DEBUG

  #endif 

  // not handling
} 
/**
 * ex. store inst.
 * deal with 16 byte value
 */
extern "C" __attribute__((visibility("default")))
void  __dfsan_hook2_int128 (unsigned id, dfsan_label value_label, dfsan_label ptr_label) {
  
  #ifdef DTAINT_DEBUG

  #endif 

  // not handling
}
/**
 * ex.memset
 */
extern "C" __attribute__((visibility("default")))
void __dfsan_hook3 (unsigned id, dfsan_label ptr_label, dfsan_label c_label, 
  dfsan_label size_label) {
  
  #ifdef DTAINT_DEBUG
  fprintf(stderr, "__dfsan_hook3: ptr_label: %u c_label: %u size_label: %u\n", 
    ptr_label, c_label, size_label);
  #endif 

  if (unlikely(!__afl_dtaint_map) || id >= DTAINT_MAP_W) return;

  unsigned hits;
  if (!__afl_dtaint_map->headers[id].type) {
    
    hits = 0;
    __afl_dtaint_map->headers[id].hits = 1;
    __afl_dtaint_map->headers[id].type = DFSH_HOOK3;

  }
  else {
    
    hits = __afl_dtaint_map->headers[id].hits++;

  }

  if (ptr_label == 0 && c_label  == 0 && size_label == 0) return;

  hits &= DTAINT_MAP_H - 1;

  __afl_dtaint_map->log[id][hits].__label_t.dst_label = ptr_label;
  __afl_dtaint_map->log[id][hits].__label_t.value_label = c_label;
  __afl_dtaint_map->log[id][hits].__label_t.size_label = size_label;

  if (ptr_label != 0 && __afl_dtaint_map->label_info[ptr_label].num == 0) offset_cpy(ptr_label);
  if (c_label != 0 && __afl_dtaint_map->label_info[c_label].num == 0) offset_cpy(c_label);
  if (size_label != 0 && __afl_dtaint_map->label_info[size_label].num == 0) offset_cpy(size_label);

}
/**
 * memcpy
 */
extern "C" __attribute__((visibility("default")))
void __dfsan_hook4 (unsigned id, dfsan_label dst_label, dfsan_label src_label, 
  dfsan_label size_label) {

  #ifdef DTAINT_DEBUG
  fprintf(stderr, "__dfsan_hook4: dst_label: %u src_label: %u size_label: %u\n", 
    dst_label, src_label, size_label);
  #endif 

  if (unlikely(!__afl_dtaint_map) || id >= DTAINT_MAP_W) return;

  unsigned hits;
  if (!__afl_dtaint_map->headers[id].type) {
    
    hits = 0;
    __afl_dtaint_map->headers[id].hits = 1;
    __afl_dtaint_map->headers[id].type = DFSH_HOOK4;

  }
  else {
    
    hits = __afl_dtaint_map->headers[id].hits++;

  }

  if (dst_label == 0 && src_label == 0 && size_label) return;

  hits &= DTAINT_MAP_H - 1;

  __afl_dtaint_map->log[id][hits].__label_t.dst_label = dst_label;
  __afl_dtaint_map->log[id][hits].__label_t.src_label = src_label;
  __afl_dtaint_map->log[id][hits].__label_t.size_label = size_label;

  if (dst_label != 0 && __afl_dtaint_map->label_info[dst_label].num == 0) offset_cpy(dst_label);
  if (src_label != 0 && __afl_dtaint_map->label_info[src_label].num == 0) offset_cpy(src_label);
  if (size_label != 0 && __afl_dtaint_map->label_info[size_label].num == 0) offset_cpy(size_label);

}
/**
 * ex. malloc
 */ 
extern "C" __attribute__((visibility("default")))
void __dfsan_hook5 (unsigned id, dfsan_label size_label) {
  
  #ifdef DTAINT_DEBUG
  fprintf(stderr, "__dfsan_hook5: size_label: %u\n", size_label);
  #endif 
  
  if (unlikely(!__afl_dtaint_map) || id >= DTAINT_MAP_W) return;

  unsigned hits;
  if (!__afl_dtaint_map->headers[id].type) {
    
    hits = 0;
    __afl_dtaint_map->headers[id].hits = 1;
    __afl_dtaint_map->headers[id].type = DFSH_HOOK5;

  }
  else {
    
    hits = __afl_dtaint_map->headers[id].hits++;

  }

  if (size_label == 0) return;

  hits &= DTAINT_MAP_H - 1;

  __afl_dtaint_map->log[id][hits].__label_t.size_label = size_label;
  if (size_label != 0 && __afl_dtaint_map->label_info[size_label].num == 0) offset_cpy(size_label);
  
}
/**
 *  ex. free
 */
extern "C" __attribute__((visibility("default")))
void __dfsan_hook6 (unsigned id, dfsan_label ptr_label) {
  
  #ifdef DTAINT_DEBUG
  fprintf(stderr, "__dfsan_hook6: ptr_label: %u\n", ptr_label);
  #endif 

  if (unlikely(!__afl_dtaint_map) || id >= DTAINT_MAP_W) return;

  unsigned hits;
  if (!__afl_dtaint_map->headers[id].type) {
    
    hits = 0;
    __afl_dtaint_map->headers[id].hits = 1;
    __afl_dtaint_map->headers[id].type = DFSH_HOOK6;

  }
  else {
    
    hits = __afl_dtaint_map->headers[id].hits++;

  }

  if (ptr_label == 0) return;

  hits &= DTAINT_MAP_H - 1;

  __afl_dtaint_map->log[id][hits].__label_t.src_label = ptr_label;
  if (ptr_label != 0 && __afl_dtaint_map->label_info[ptr_label].num == 0) offset_cpy(ptr_label);

}

/**
 * ex. realloc
 */
extern "C" __attribute__((visibility("default")))
void __dfsan_hook7 (unsigned id, dfsan_label ptr_label, dfsan_label size_label) {

  #ifdef DTAINT_DEBUG
  fprintf(stderr, "__dfsan_hook7: ptr_label: %u size_label: %u\n", ptr_label, size_label);
  #endif 
  
  if (unlikely(!__afl_dtaint_map) || id >= DTAINT_MAP_W) return;

  unsigned hits;
  if (!__afl_dtaint_map->headers[id].type) {
    
    hits = 0;
    __afl_dtaint_map->headers[id].hits = 1;
    __afl_dtaint_map->headers[id].type = DFSH_HOOK7;

  }
  else {
    
    hits = __afl_dtaint_map->headers[id].hits++;

  }

  if (ptr_label == 0 && size_label == 0) return;

  hits &= DTAINT_MAP_H - 1;

  __afl_dtaint_map->log[id][hits].__label_t.src_label = ptr_label;
  __afl_dtaint_map->log[id][hits].__label_t.size_label = size_label;

  if (ptr_label != 0 && __afl_dtaint_map->label_info[ptr_label].num == 0) offset_cpy(ptr_label);
  if (size_label != 0 && __afl_dtaint_map->label_info[size_label].num == 0) offset_cpy(size_label);

}

/**
 * ex. get_element_ptr inst.
 */
extern "C" __attribute__((visibility("default")))
void __dfsan_va_arg_hook1 (unsigned id, dfsan_label ptr_label, unsigned num_of_idx, ...) {

  #ifdef DTAINT_DEBUG
  va_list args;
  int size, logged;
  
  fprintf(stderr, "__dfsan_vararg_hook1: id: %u ptr_label: %u, num: %u\n", 
  id, ptr_label, num_of_idx);

  if (num_of_idx > DTAINT_MAXiMUM_IDX_NUM)
    logged = DTAINT_MAXiMUM_IDX_NUM;
  else
    logged = num_of_idx;

  va_start(args, num_of_idx);
  for(int j = 0; j < logged; j++) {
    fprintf(stderr, "idx %u ", va_arg(args, int));
  }fprintf(stderr, "\n");

  va_end(args);
  #endif

  if (unlikely(!__afl_dtaint_map) || id >= DTAINT_MAP_W) return;

  unsigned hits;
  if (!__afl_dtaint_map->headers[id].type) {
    
    hits = 0;
    __afl_dtaint_map->headers[id].hits = 1;
    __afl_dtaint_map->headers[id].type = DFSH_VARARG_HOOK1;

  }
  else {
    
    hits = __afl_dtaint_map->headers[id].hits++;

  }
  
  hits &= DTAINT_MAP_H - 1;
  
  if (num_of_idx > DTAINT_MAXiMUM_IDX_NUM)
    logged = DTAINT_MAXiMUM_IDX_NUM;
  else
    logged = num_of_idx;

  __afl_dtaint_map->log[id][hits].__va_arg_label_t.num = logged;
  __afl_dtaint_map->log[id][hits].__va_arg_label_t.ptr_label = ptr_label;
  
  if (ptr_label != 0 && __afl_dtaint_map->label_info[ptr_label].num == 0) offset_cpy(ptr_label);

  dfsan_label *p = __afl_dtaint_map->log[id][hits].__va_arg_label_t.idx_label;
  
  dfsan_label label;
  va_start(args, num_of_idx);
  for(int i = 0; i < logged; i++) {
    
    label = va_arg(args, int);
    *(p + i) = label;

    if (label != 0 && __afl_dtaint_map->label_info[label].num == 0) {
      // do we need to check label ?
      offset_cpy(label);
    }

  }
  va_end(args);

}

