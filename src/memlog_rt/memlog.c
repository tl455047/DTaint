#include <stdio.h>
#include <stdarg.h>
#include "memlog.h"
#include "types.h"

typedef unsigned __int128 uint128_t;

extern struct memlog_map *__afl_memlog_map;
/**
 * Call hook.
 * __memlog_hook1 (unsigned id, void* ptr, u8 src_type, u8 rst_type);
 * ex. load inst.
 * __memlog_hook2 (unsigned id, size_t value, int src_type, void* ptr, int rst_type);
 * ex. store inst.
 *__memlog_hook2_int128 (unsigned id, uint128_t value, int src_type, void* ptr, int rst_type);
 * deal with 16byte float point value
 * __memlog_hook3 (unsigned id, void* ptr, int c, size_t size);
 * ex. memset
 * __memlog_hook4 (unsigned id, void* dst, void* src, size_t size);
 * ex. memcpy
 * __memlog_hook5 (unsigned id, int type, size_t size);
 * ex. alloca inst.
 * __memlog_hook6 (unsigned id, size_t size);
 * ex. malloc
 * __memlog_hook7 (unsigned id, void* ptr);
 * ex. free
 * __memlog_hook8 (unsigned id, void* ptr, size_t size);
 * ex. realloc
 * __memlog_vararg_hook1 (unsigned id, void* ptr, size_t size, size_t num_of_args, ...);
 * ex. get_element_ptr inst.
 * 
 * 
 */
#ifdef MEMLOG_DEBUG

void __memlog_debug_output() {
  
  for(int i = 0; i < MEMLOG_MAP_W; i++) {
    
    if (!__afl_memlog_map->headers[i].hits) continue;

    fprintf(stderr, "header: id: %u hits: %u src_shape: %u rst_shape: %u type: %u\n", 
      __afl_memlog_map->headers[i].id, 
      __afl_memlog_map->headers[i].hits,
      __afl_memlog_map->headers[i].src_shape,
      __afl_memlog_map->headers[i].rst_shape,
      __afl_memlog_map->headers[i].type);
    
    switch(__afl_memlog_map->headers[i].type) {
      case HT_VARARG_HOOK1:
        fprintf(stderr, "hook va arg log: offset: %u num: %u ptr: %p\n",
          __afl_memlog_map->log[i][0].__hook_va_arg.offset,
          __afl_memlog_map->log[i][0].__hook_va_arg.num,
          __afl_memlog_map->log[i][0].__hook_va_arg.ptr);
        struct hook_va_arg_idx* p = &__afl_memlog_map->
          va_arg_idx[ __afl_memlog_map->log[i][0].__hook_va_arg.offset];
        for(int j = 0; j < __afl_memlog_map->log[i][0].__hook_va_arg.num; j++) {
          fprintf(stderr, "idx: %lld type: %u ",
          (p + j)->idx,
          (p + j)->type);
        }fprintf(stderr, "\n");
        break;
      default:
        fprintf(stderr, "hook log: type: %u dst: %p src: %p value: %lld size: %lld\n",
          __afl_memlog_map->headers[i].type,
          __afl_memlog_map->log[i][0].__hook_op.dst,
          __afl_memlog_map->log[i][0].__hook_op.src,
          __afl_memlog_map->log[i][0].__hook_op.value,
          __afl_memlog_map->log[i][0].__hook_op.size);
        break;
    }

  }
}

__attribute__((constructor)) 
void __memlog_debug_init() {
      
  fprintf(stderr, "__memlog_debug_init\n");

  if(!__afl_memlog_map) {
      __afl_memlog_map = (struct memlog_map *)malloc(sizeof(struct memlog_map));
  }

}

__attribute__((destructor)) 
void __memlog_debug_fini() {

  fprintf(stderr, "__memlog_debug_fini\n");

  __memlog_debug_output();

  if(__afl_memlog_map) 
    free(__afl_memlog_map);

}

void __memlog_hook_debug(int num1, void* num2, int num3, int num4) {
 
  fprintf(stderr, "num1: %d num2: %p num3: %d num4: %d\n", num1, num2, num3, num4);

}

#endif

/**
 * ex. load inst.
 */
__attribute__((visibility("default")))
void __memlog_hook1(unsigned id, void* ptr, int src_type, int rst_type) {
  
  #ifdef MEMLOG_DEBUG
  fprintf(stderr, "__memlog_hook1: id: %u ptr: %p src_type: %d rst type:\
  %d\n", id, ptr, src_type, rst_type);
  #endif
  
  if (unlikely(!__afl_memlog_map)) return;

  unsigned hits;
  if (!__afl_memlog_map->headers[id].type) {
    
    hits = 0;
    __afl_memlog_map->headers[id].hits = 1;
    __afl_memlog_map->headers[id].type = HT_HOOK1;
    
    __afl_memlog_map->headers[id].src_shape = src_type;
    __afl_memlog_map->headers[id].rst_shape = rst_type;
    
  }
  else {

    hits = __afl_memlog_map->headers[id].hits++;
  
  }
  
  hits &= MEMLOG_MAP_H - 1;
  __afl_memlog_map->log[id][hits].__hook_op.src = ptr;

}

/**
 * ex. store inst.
 */
__attribute__((visibility("default")))
void __memlog_hook2(unsigned id, size_t value, int src_type, void* ptr, int rst_type) {
  
  #ifdef MEMLOG_DEBUG
  fprintf(stderr, "__memlog_hook2: id: %u value: %x src_type: %d\
  ptr: %p rst_type: %d\n", id, value, src_type, ptr, rst_type);
  #endif
  
  if (unlikely(!__afl_memlog_map)) return;

  unsigned hits;
  if (!__afl_memlog_map->headers[id].type) {
    
    hits = 0;
    __afl_memlog_map->headers[id].hits = 1;
    __afl_memlog_map->headers[id].type = HT_HOOK2;
  
    __afl_memlog_map->headers[id].src_shape = src_type;
    __afl_memlog_map->headers[id].rst_shape = rst_type;

  }
  else {
    
    hits = __afl_memlog_map->headers[id].hits++;

  }
  
  hits &= MEMLOG_MAP_H - 1;
  __afl_memlog_map->log[id][hits].__hook_op.dst = ptr;
  __afl_memlog_map->log[id][hits].__hook_op.value = value;

}

/**
 * Deal with floating point 16 byte value.
 */ 
void __memlog_hook2_int128(unsigned id, uint128_t value, int src_type, void* ptr, int rst_type) {
  
  #ifdef MEMLOG_DEBUG
  fprintf(stderr, "__memlog_hook2_int128: id: %u value: %x src_type: %d\
  ptr: %p rst_type: %d\n", id, value, src_type, ptr, rst_type);
  #endif
  
  if (unlikely(!__afl_memlog_map)) return;

  unsigned hits;
  if (!__afl_memlog_map->headers[id].type) {
    
    hits = 0;
    __afl_memlog_map->headers[id].hits = 1;
    __afl_memlog_map->headers[id].type = HT_HOOK2_INT128;
  
    __afl_memlog_map->headers[id].src_shape = src_type;
    __afl_memlog_map->headers[id].rst_shape = rst_type;

  }
  else {
    
    hits = __afl_memlog_map->headers[id].hits++;

  }
  
  hits &= MEMLOG_MAP_H - 1;
  
  __afl_memlog_map->log[id][hits].__hook_op.dst = ptr;
  __afl_memlog_map->log[id][hits].__hook_op.value = (uint64_t)value;
  __afl_memlog_map->log[id][hits].__hook_op.value_128 = (uint64_t)(value >> 64);

}

/**
 * ex. memset
 */
__attribute__((visibility("default")))
void __memlog_hook3(unsigned id, void* ptr, int c, size_t size) {
  
  #ifdef MEMLOG_DEBUG
  fprintf(stderr, "__memlog_hook3: id: %u ptr: %p c: %d size: %lld\n", id, ptr, c, size);
  #endif
  
  if (unlikely(!__afl_memlog_map)) return;

  unsigned hits;
  if (!__afl_memlog_map->headers[id].type) {
    
    hits = 0;
    __afl_memlog_map->headers[id].hits = 1;
    __afl_memlog_map->headers[id].type = HT_HOOK3;
  
    __afl_memlog_map->headers[id].src_shape = 1;
    __afl_memlog_map->headers[id].rst_shape = 1;

  }
  else {
    
    hits = __afl_memlog_map->headers[id].hits++;

  }
  
  hits &= MEMLOG_MAP_H - 1;
  __afl_memlog_map->log[id][hits].__hook_op.dst = ptr;
  __afl_memlog_map->log[id][hits].__hook_op.value = c;
  __afl_memlog_map->log[id][hits].__hook_op.size = size;

}

/**
 * ex. memcpy
 */
__attribute__((visibility("default")))
void __memlog_hook4(unsigned id, void* dst, void* src, size_t size) {
  
  #ifdef MEMLOG_DEBUG
  fprintf(stderr, "__memlog_hook4: id: %u dst: %p src: %p size: %lld\n", id, dst, src, size);
  #endif
  
  if (unlikely(!__afl_memlog_map)) return;

  unsigned hits;
  if (!__afl_memlog_map->headers[id].type) {
    
    hits = 0;
    __afl_memlog_map->headers[id].hits = 1;
    __afl_memlog_map->headers[id].type = HT_HOOK4;
  
    __afl_memlog_map->headers[id].src_shape = 1;
    __afl_memlog_map->headers[id].rst_shape = 1;

  }
  else {
    
    hits = __afl_memlog_map->headers[id].hits++;

  }

  hits &= MEMLOG_MAP_H - 1;
  __afl_memlog_map->log[id][hits].__hook_op.dst = dst;
  __afl_memlog_map->log[id][hits].__hook_op.src = src;
  __afl_memlog_map->log[id][hits].__hook_op.size = size;

}

/**
 * ex. alloca inst.
 */
__attribute__((visibility("default")))
void __memlog_hook5(unsigned id, int type, size_t size) {
  
  #ifdef MEMLOG_DEBUG
  fprintf(stderr, "__memlog_hook5: id: %u type: %d size: %lld\n", id, type, size);
  #endif
  
  if (unlikely(!__afl_memlog_map)) return;

  unsigned hits;
  if (!__afl_memlog_map->headers[id].type) {
  
    hits = 0;
    __afl_memlog_map->headers[id].hits = 1;
    __afl_memlog_map->headers[id].type = HT_HOOK5;
  
    __afl_memlog_map->headers[id].src_shape = 1;
    __afl_memlog_map->headers[id].rst_shape = type;

  }
  else {
    
    hits = __afl_memlog_map->headers[id].hits++;
  
  }
  
  hits &= MEMLOG_MAP_H - 1;
  __afl_memlog_map->log[id][hits].__hook_op.size = size;

}

/**
 * ex. malloc
 */
__attribute__((visibility("default")))
void __memlog_hook6(unsigned id, size_t size) {

  #ifdef MEMLOG_DEBUG
  fprintf(stderr, "__memlog_hook6: id: %u size: %lld\n", id, size);
  #endif

  if (unlikely(!__afl_memlog_map)) return;

  unsigned hits;
  if (!__afl_memlog_map->headers[id].type) {
    
    hits = 0;
    __afl_memlog_map->headers[id].hits = 1;
    __afl_memlog_map->headers[id].type = HT_HOOK6;
  
    __afl_memlog_map->headers[id].src_shape = 1;
    __afl_memlog_map->headers[id].rst_shape = 1;

  }
  else {
    
    hits = __afl_memlog_map->headers[id].hits++;

  }

  hits &= MEMLOG_MAP_H - 1;
  __afl_memlog_map->log[id][hits].__hook_op.size = size;
  
}

/**
 * ex. free
 */
__attribute__((visibility("default")))
void __memlog_hook7(unsigned id, void* ptr) {
  
  #ifdef MEMLOG_DEBUG
  fprintf(stderr, "__memlog_hook7: id: %u ptr: %p\n", id, ptr);
  #endif

  if (unlikely(!__afl_memlog_map)) return;

  unsigned hits;
  if(!__afl_memlog_map->headers[id].type) {
  
    hits = 0;
    __afl_memlog_map->headers[id].hits = 1;
    __afl_memlog_map->headers[id].type = HT_HOOK7;

  }
  else {
    
    hits = __afl_memlog_map->headers[id].hits++;
  
  }

  hits &= MEMLOG_MAP_H - 1;
  __afl_memlog_map->log[id][hits].__hook_op.src = ptr;

}

/**
 * ex. realloc
 */
__attribute__((visibility("default")))
void __memlog_hook8(unsigned id, void* ptr, size_t size) {
  
  #ifdef MEMLOG_DEBUG
  fprintf(stderr, "__memlog_hook8: id: %u ptr: %p size: %lld\n", id, ptr, size);
  #endif

  if (unlikely(!__afl_memlog_map)) return;

  unsigned hits;
  if (!__afl_memlog_map->headers[id].type) {
  
    hits = 0;
    __afl_memlog_map->headers[id].hits = 1;
    __afl_memlog_map->headers[id].type = HT_HOOK8;
  
    __afl_memlog_map->headers[id].src_shape = 1;
    __afl_memlog_map->headers[id].rst_shape = 1;

  }
  else {
    
    hits = __afl_memlog_map->headers[id].hits++;

  }

  hits &= MEMLOG_MAP_H - 1;
  __afl_memlog_map->log[id][hits].__hook_op.src = ptr;
  __afl_memlog_map->log[id][hits].__hook_op.size = size;

}

/**
 * ex. get_element_ptr inst.
 */
__attribute__((visibility("default")))
void __memlog_va_arg_hook1(unsigned id, void* ptr, int src_type, int rst_type, int num_of_idx, ...) {
  //deal with vararg
  va_list args;
  int size;
  #ifdef MEMLOG_DEBUG
  va_start(args, num_of_idx);
  fprintf(stderr, "__memlog_vararg_hook1: id: %u ptr: %p src_type: %d\
  rst_type: %d offset: %d ", id, ptr, src_type, rst_type, num_of_idx);
  for(int j = 0; j < num_of_idx; j++) {
    size = va_arg(args, int);
    
    if (size <= sizeof(int))
      fprintf(stderr, "%d ", va_arg(args, int));
    else if (size <= sizeof(long long))
      fprintf(stderr, "%lld ", va_arg(args, long long));

  }fprintf(stderr, "\n");

  va_end(args);
  #endif

  if (unlikely(!__afl_memlog_map)) return;

  unsigned hits;
  if (!__afl_memlog_map->headers[id].type) {
    
    hits = 0;
    __afl_memlog_map->headers[id].hits = 1;
    __afl_memlog_map->headers[id].type = HT_VARARG_HOOK1;

    __afl_memlog_map->headers[id].src_shape = src_type;
    __afl_memlog_map->headers[id].rst_shape = rst_type;

  }
  else {
    
    hits = __afl_memlog_map->headers[id].hits++;

  }
  
  hits &= MEMLOG_MAP_H - 1;
  // Does it need to store total length ?
  __afl_memlog_map->log[id][hits].__hook_va_arg.num = num_of_idx;
  __afl_memlog_map->log[id][hits].__hook_va_arg.ptr = ptr;
  __afl_memlog_map->log[id][hits].__hook_va_arg.offset = 
  __afl_memlog_map->current_pos;
  
  // store idx 
  /*struct hook_va_arg_idx *p = 
    &__afl_memlog_map->va_arg_idx[__afl_memlog_map->current_pos];

  va_start(args, num_of_idx);
  for(int i = 0; i < num_of_idx; i++) {
    
    size = va_arg(args, int);
    (p + i)->type = size; 
    
    if (size <= sizeof(int))
      (p + i)->idx = va_arg(args, int);
    else if (size <= sizeof(long long))
      (p + i)->idx = va_arg(args, long long);
    
  }
  va_end(args);

  __afl_memlog_map->current_pos += num_of_idx;*/

}
