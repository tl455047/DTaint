#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include "memlog.h"
#include "types.h"
#include "config.h"

#define XXH_INLINE_ALL
#include "xxhash.h"
#undef XXH_INLINE_ALL

#ifdef _DEBUG
u64 hash64(u8 *key, u32 len, u64 seed) {

#else
static inline u64 hash64(u8 *key, u32 len, u64 seed) {

#endif

  return XXH64(key, len, seed);

}

typedef unsigned __int128 uint128_t;

extern struct mem_map *__afl_mem_map;
extern u8 *__afl_area_ptr;
// memlog map size not real target mapsize, but this is enough
extern u32 __afl_map_size;

// use for cksum calculating
u8 *__memlog_cksum_map;
u32 __memlog_cksum_map_size;
/**
 * Call hook.
 * __memlog_hook1 (unsigned id, void* ptr, unsigned src_type, unsigned rst_type);
 * ex. load inst.
 * __memlog_hook2 (unsigned id, size_t value, unsigned src_type, void* ptr, unsigned rst_type);
 * ex. store inst.
 *__memlog_hook2_int128 (unsigned id, __int128 value, unsigned src_type, void* ptr, unsigned rst_type);
 * deal with 16 byte value
 * __memlog_hook3 (unsigned id, void* ptr, int c, size_t size);
 * ex. memset
 * __memlog_hook4 (unsigned id, void* dst, void* src, size_t size);
 * ex. memcpy
 * __memlog_hook5 (unsigned id, size_t size);
 * ex. malloc
 * __memlog_hook6 (unsigned id, void* ptr);
 * ex. free
 * __memlog_hook7 (unsigned id, void* ptr, size_t size);
 * ex. realloc
 * __memlog_get_element_ptr_hook (unsigned id, void* ptr, unsigned src_type, 
 * unsigned rst_type, unsigned num_of_idx, ...);
 * ex. get_element_ptr inst.
 * 
 */
#ifdef MEMLOG_DEBUG

void __memlog_debug_output() {
  
  /*struct hook_va_arg_operand *__hook_va_arg;
  for (int i = 0; i < MEM_MAP_W; i++) {
    
    if (!__afl_mem_map->headers[i].hits) continue;
  
    fprintf(stderr, "header: id: %u hits: %u type: %u\n", 
      i, 
      __afl_mem_map->headers[i].hits,
      __afl_mem_map->headers[i].type);
    switch (__afl_mem_map->headers[i].type) {
      case HT_GEP_HOOK:
          
        fprintf(stderr, "header: id: %u hits: %u type: %u\n", 
          i, 
          __afl_mem_map->headers[i].hits,
          __afl_mem_map->headers[i].type);
        fprintf(stderr, "hook va arg log: num: %p ptr: %p\n",
          __afl_mem_map->log[i][0].__hook_va_arg.num,
          __afl_mem_map->log[i][0].__hook_va_arg.ptr);

        __hook_va_arg = &__afl_mem_map->log[i][0].__hook_va_arg;
        for (int j = 0; j < __afl_mem_map->log[i][0].__hook_va_arg.num; j++) {
          fprintf(stderr, "%u ", __hook_va_arg->idx[j]);
        }fprintf(stderr, "\n");
        break;
      case HT_HOOK3:
      case HT_HOOK4:
      case HT_HOOK5:
      case HT_HOOK6:
      case HT_HOOK7: {
        fprintf(stderr, "header: id: %u hits: %u type: %u\n", 
          i, 
          __afl_mem_map->headers[i].hits,
          __afl_mem_map->headers[i].type);
        break;
      }
      default:
        break;
    }

  }*/

}

__attribute__((constructor)) 
void __memlog_debug_init() {
      
  /*fprintf(stderr, "__memlog_debug_init\n");

  if(!__afl_mem_map) {
      __afl_mem_map = (struct mem_map *)malloc(sizeof(struct mem_map));
    memset(__afl_mem_map, 0, sizeof(struct mem_map));
  }*/

}

__attribute__((destructor)) 
void __memlog_debug_fini() {

  /*fprintf(stderr, "__memlog_debug_fini\n");
  if (__afl_mem_map) 
    __memlog_debug_output();
  
  if(__afl_mem_map && !getenv(MEMLOG_SHM_ENV_VAR)) {
    free(__afl_mem_map);
    __afl_mem_map = NULL;
  }*/

}

void __memlog_hook_debug(u32 id) {
  
  fprintf(stderr, "__memlog_hook_debug id: %u\n", id);

  if (unlikely(!__afl_mem_map)) return;

  unsigned hits;
  if (!__afl_mem_map->headers[id].type) {
    
    hits = 0;
    __afl_mem_map->headers[id].hits = 1;
    __afl_mem_map->headers[id].type = HT_GEP_HOOK;
    
  }
  else {

    hits = __afl_mem_map->headers[id].hits++;
  
  }
}
#endif


__attribute((constructor(4)))
void __memlog_set_cksum_map() {

  if (unlikely(!__afl_mem_map)) return;

  if (__afl_map_size < MEM_MAP_W) {
    // seems the map size is smaller
    // we can use afl bitmap to calculate control flow cksum
    __memlog_cksum_map = __afl_area_ptr;
    __memlog_cksum_map_size = __afl_map_size;

  }
  else {
    // size of afl bitmap is larger than memlog map size
    // let's use memlog map to calculate control flow cksum
    __memlog_cksum_map = __afl_mem_map->hits;
    __memlog_cksum_map_size = MEM_MAP_W;

  }

}
/**
 * ex. load inst.
 */
__attribute__((visibility("default")))
void __memlog_hook1(u32 id, void* ptr, u32 src_type, u32 rst_type) {
  
  #ifdef MEMLOG_DEBUG
  /*fprintf(stderr, "__memlog_hook1: id: %u ptr: %p src_type: %u rst type:\
  %u value: ", id, ptr, src_type, rst_type);
  if (src_type < sizeof(int64_t)) 
    fprintf(stderr, "%u\n", *((int *)ptr));
  else if (src_type < sizeof(__int128))
    fprintf(stderr, "%llu\n", *((int64_t *)ptr));
  else {

    uint128_t val = *((uint128_t *)ptr);
    fprintf(stderr, "upper: %llu lower: %llu\n", (uint64_t )val, (uint64_t)(val >> 64));

  }*/
  #endif
  
  if (unlikely(!__afl_mem_map)) return;

  unsigned hits;
  if (!__afl_mem_map->headers[id].type) {
    
    hits = 0;
    __afl_mem_map->headers[id].hits = 1;
    __afl_mem_map->headers[id].type = HT_HOOK1;
    
    __afl_mem_map->headers[id].src_shape = src_type;
    __afl_mem_map->headers[id].rst_shape = rst_type;
    
  }
  else {

    hits = __afl_mem_map->headers[id].hits++;
  
  }

  hits &= MEM_MAP_H - 1;
  if (src_type < sizeof(int64_t)) {

    __afl_mem_map->log[id][hits].__hook_op.value = *((unsigned *)ptr);
  
  }
  else if (src_type < sizeof(uint128_t)){

    __afl_mem_map->log[id][hits].__hook_op.value = *((uint64_t *)ptr);

  }
  else {

    uint128_t val = *((uint128_t *)ptr);
    __afl_mem_map->log[id][hits].__hook_op.value = (uint64_t )val;
    __afl_mem_map->log[id][hits].__hook_op.value_128 = (uint64_t)(val >> 64);

  }

  __afl_mem_map->log[id][hits].__hook_op.src = ptr;

}

/**
 * ex. store inst.
 */
__attribute__((visibility("default")))
void __memlog_hook2(u32 id, size_t value, u32 src_type, void* ptr, 
  unsigned rst_type) {
  
  #ifdef MEMLOG_DEBUG
  if (id == 870) 
  /*fprintf(stderr, "__memlog_hook2: id: %u value: %x src_type: %u\
  ptr: %p rst_type: %u\n", id, value, src_type, ptr, rst_type);*/
  //fprintf(stderr, "__memlog_hook2 id: %u\n", id);
  #endif
  
  if (unlikely(!__afl_mem_map)) return;

  unsigned hits;
  if (!__afl_mem_map->headers[id].type) {
    
    hits = 0;
    __afl_mem_map->headers[id].hits = 1;
    __afl_mem_map->headers[id].type = HT_HOOK2;
  
    __afl_mem_map->headers[id].src_shape = src_type;
    __afl_mem_map->headers[id].rst_shape = rst_type;

  }
  else {
    
    hits = __afl_mem_map->headers[id].hits++;

  }
  
  hits &= MEM_MAP_H - 1;
  __afl_mem_map->log[id][hits].__hook_op.dst = ptr;
  __afl_mem_map->log[id][hits].__hook_op.value = value;

}

/**
 * Deal with floating point 16 byte value.
 */ 
void __memlog_hook2_int128(u32 id, uint128_t value, u32 src_type,
  void* ptr, unsigned rst_type) {
  
  #ifdef MEMLOG_DEBUG
  /*fprintf(stderr, "__memlog_hook2_int128: id: %u upper value: %p lower value: %p\
  src_type: %u ptr: %p rst_type: %u\n", id, (int64_t)(value >> 64), 
  (int64_t)value, src_type, ptr, rst_type);*/
  #endif
  
  if (unlikely(!__afl_mem_map)) return;

  unsigned hits;
  if (!__afl_mem_map->headers[id].type) {
    
    hits = 0;
    __afl_mem_map->headers[id].hits = 1;
    __afl_mem_map->headers[id].type = HT_HOOK2_INT128;
  
    __afl_mem_map->headers[id].src_shape = src_type;
    __afl_mem_map->headers[id].rst_shape = rst_type;

  }
  else {
    
    hits = __afl_mem_map->headers[id].hits++;

  }
  
  hits &= MEM_MAP_H - 1;
  __afl_mem_map->log[id][hits].__hook_op.dst = ptr;
  __afl_mem_map->log[id][hits].__hook_op.value = (uint64_t)value;
  __afl_mem_map->log[id][hits].__hook_op.value_128 = (uint64_t)(value >> 64);

}

/**
 * ex. memset
 */
__attribute__((visibility("default")))
void __memlog_hook3(u32 id, void* ptr, int c, u64 size) {
  
  #ifdef MEMLOG_DEBUG
  fprintf(stderr, "__memlog_hook3: id: %u ptr: %p c: %u size: %llu\n", id, ptr, 
    c, size);
  #endif
  
  if (unlikely(!__afl_mem_map)) return;

  unsigned hits;
  if (!__afl_mem_map->headers[id].type) {
    
    hits = 0;
    __afl_mem_map->headers[id].hits = 1;
    __afl_mem_map->headers[id].type = HT_HOOK3;
  
    __afl_mem_map->headers[id].src_shape = 1;
    __afl_mem_map->headers[id].rst_shape = 1;

    // used for hash calculating
    __afl_mem_map->hits[id] = 1;

  }
  else {
    
    hits = __afl_mem_map->headers[id].hits++;

    // used for hash calculating
    __afl_mem_map->hits[id]++;

  }
  
  hits &= MEM_MAP_H - 1;

  // calculate current memlog map header hash
  // can be used to distinguish different path
  __afl_mem_map->cksum[id][hits] = hash64((void *)__memlog_cksum_map, __memlog_cksum_map_size, HASH_CONST);

  __afl_mem_map->log[id][hits].__hook_op.dst = ptr;
  __afl_mem_map->log[id][hits].__hook_op.value = c;
  __afl_mem_map->log[id][hits].__hook_op.size = size;

}

/**
 * ex. memcpy
 */
__attribute__((visibility("default")))
void __memlog_hook4(u32 id, void* dst, void* src, u64 size) {
  
  #ifdef MEMLOG_DEBUG
  fprintf(stderr, "__memlog_hook4: id: %u dst: %p src: %p size: %llu\n", id, 
    dst, src, size);
  #endif
  
  if (unlikely(!__afl_mem_map)) return;

  unsigned hits;
  if (!__afl_mem_map->headers[id].type) {
    
    hits = 0;
    __afl_mem_map->headers[id].hits = 1;
    __afl_mem_map->headers[id].type = HT_HOOK4;
  
    __afl_mem_map->headers[id].src_shape = 1;
    __afl_mem_map->headers[id].rst_shape = 1;

    // used for hash calculating
    __afl_mem_map->hits[id] = 1;
 
  }
  else {
    
    hits = __afl_mem_map->headers[id].hits++;

    // used for hash calculating
    __afl_mem_map->hits[id]++;
    
  }

  hits &= MEM_MAP_H - 1;

  // calculate current memlog map header hash
  // can be used to distinguish different path
  __afl_mem_map->cksum[id][hits] = hash64((void *)__memlog_cksum_map, __memlog_cksum_map_size, HASH_CONST);

  __afl_mem_map->log[id][hits].__hook_op.dst = dst;
  __afl_mem_map->log[id][hits].__hook_op.src = src;
  __afl_mem_map->log[id][hits].__hook_op.size = size;

}

/**
 * ex. malloc
 */
__attribute__((visibility("default")))
void __memlog_hook5(u32 id, u64 size) {

  #ifdef MEMLOG_DEBUG
  fprintf(stderr, "__memlog_hook5: id: %u size: %llu\n", id, size);
  #endif

  if (unlikely(!__afl_mem_map)) return;

  unsigned hits;
  if (!__afl_mem_map->headers[id].type) {
    
    hits = 0;
    __afl_mem_map->headers[id].hits = 1;
    __afl_mem_map->headers[id].type = HT_HOOK5;
  
    __afl_mem_map->headers[id].src_shape = 1;
    __afl_mem_map->headers[id].rst_shape = 1;

    // used for hash calculating
    __afl_mem_map->hits[id] = 1;

  }
  else {
    
    hits = __afl_mem_map->headers[id].hits++;

    // used for hash calculating
    __afl_mem_map->hits[id]++;

  }

  hits &= MEM_MAP_H - 1;

  // calculate current memlog map header hash
  // can be used to distinguish different path
  __afl_mem_map->cksum[id][hits] = hash64((void *)__memlog_cksum_map, __memlog_cksum_map_size, HASH_CONST);

  __afl_mem_map->log[id][hits].__hook_op.size = size;
  
}

/**
 * ex. free
 */
__attribute__((visibility("default")))
void __memlog_hook6(u32 id, void* ptr) {
  
  #ifdef MEMLOG_DEBUG
  fprintf(stderr, "__memlog_hook6: id: %u ptr: %p\n", id, ptr);
  #endif

  if (unlikely(!__afl_mem_map)) return;

  unsigned hits;
  if(!__afl_mem_map->headers[id].type) {
  
    hits = 0;
    __afl_mem_map->headers[id].hits = 1;
    __afl_mem_map->headers[id].type = HT_HOOK6;

    // used for hash calculating
    __afl_mem_map->hits[id] = 1;
  }
  else {
    
    hits = __afl_mem_map->headers[id].hits++;

    // used for hash calculating
    __afl_mem_map->hits[id]++;
  }

  hits &= MEM_MAP_H - 1;

  // calculate current memlog map header hash
  // can be used to distinguish different path
  __afl_mem_map->cksum[id][hits] = hash64((void *)__memlog_cksum_map, __memlog_cksum_map_size, HASH_CONST);

  __afl_mem_map->log[id][hits].__hook_op.src = ptr;

}

/**
 * ex. realloc
 */
__attribute__((visibility("default")))
void __memlog_hook7(u32 id, void* ptr, u64 size) {
  
  #ifdef MEMLOG_DEBUG
  fprintf(stderr, "__memlog_hook7: id: %u ptr: %p size: %llu\n", id, ptr, size);
  #endif

  if (unlikely(!__afl_mem_map)) return;

  unsigned hits;
  if (!__afl_mem_map->headers[id].type) {
  
    hits = 0;
    __afl_mem_map->headers[id].hits = 1;
    __afl_mem_map->headers[id].type = HT_HOOK7;
  
    __afl_mem_map->headers[id].src_shape = 1;
    __afl_mem_map->headers[id].rst_shape = 1;

    // used for hash calculating
    __afl_mem_map->hits[id] = 1;
  }
  else {
    
    hits = __afl_mem_map->headers[id].hits++;

    // used for hash calculating
    __afl_mem_map->hits[id]++;

  }

  hits &= MEM_MAP_H - 1;

  // calculate current memlog map header hash
  // can be used to distinguish different path
  __afl_mem_map->cksum[id][hits] = hash64((void *)__memlog_cksum_map, __memlog_cksum_map_size, HASH_CONST);

  __afl_mem_map->log[id][hits].__hook_op.src = ptr;
  __afl_mem_map->log[id][hits].__hook_op.size = size;

}

/**
 * ex. get_element_ptr inst.
 */
__attribute__((visibility("default")))
void __memlog_get_element_ptr_hook(u32 id, void* ptr, u32 src_type, u32 rst_type, 
  u32 num_of_idx, ...) {
  //deal with vararg
  va_list args;
  u32 logged;
  struct hook_va_arg_operand *__hook_va_arg;
  #ifdef MEMLOG_DEBUG
  if (num_of_idx > MEM_MAP_MAX_IDX)
    logged = MEM_MAP_MAX_IDX;
  else
    logged = num_of_idx;

  fprintf(stderr, "__memlog_get_element_ptr_hook: id: %u src_type: %u\
    rst_type: %u num: %u ", id, src_type, rst_type, num_of_idx);
  
  va_start(args, num_of_idx);
  for(int j = 0; j < logged; j++) {
    
    fprintf(stderr, "idx: %llu ", va_arg(args, u64));

  }fprintf(stderr, "\n");
  va_end(args);
 
  #endif

  if (unlikely(!__afl_mem_map)) return;

  unsigned hits;
  if (!__afl_mem_map->headers[id].type) {
    
    hits = 0;
    __afl_mem_map->headers[id].hits = 1;
    __afl_mem_map->headers[id].type = HT_GEP_HOOK;

    __afl_mem_map->headers[id].src_shape = src_type;
    __afl_mem_map->headers[id].rst_shape = rst_type;

    // used for hash calculating
    __afl_mem_map->hits[id] = 1;

  }
  else {
    
    hits = __afl_mem_map->headers[id].hits++;

    // used for hash calculating
    __afl_mem_map->hits[id]++;

  }
  
  hits &= MEM_MAP_H - 1;

  // calculate current memlog map header hash
  // can be used to distinguish different path
  __afl_mem_map->cksum[id][hits] = hash64((void *)__memlog_cksum_map, __memlog_cksum_map_size, HASH_CONST);

  if (num_of_idx > MEM_MAP_MAX_IDX)
    logged = MEM_MAP_MAX_IDX;
  else
    logged = num_of_idx;

  __hook_va_arg = &__afl_mem_map->log[id][hits].__hook_va_arg;
  __hook_va_arg->num = logged;
  __hook_va_arg->ptr = ptr;
  
  va_start(args, num_of_idx);
  for(int i = 0; i < logged; i++) {
    
    __hook_va_arg->idx[i] = va_arg(args, u64);
    
  }
  va_end(args);


}
