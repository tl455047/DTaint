#include "dfsan_hook.h"
#include "dtaint.h"
#include "types.h" 
#include <stdio.h>
#include <stdarg.h>

typedef unsigned __int128 uint128_t;

extern struct dfsan_label_info __dfsan_label_info;
extern struct dtaint_map *__afl_dtaint_map;

#ifdef DTAINT_DEBUG

void __dfsan_hook_debug_output() {
  
  for (int i = 0; i < DTAINT_MAP_W; i++) {
    
    if (!__afl_dtaint_map->headers[i].hits) continue;


  }

}

__attribute__((constructor)) 
void __dfsan_hook_debug_init() {
      
  fprintf(stderr, "__memlog_debug_init\n");

  if(!__afl_dtaint_map) {
      __afl_dtaint_map = (struct dtaint_map *)malloc(sizeof(struct dtaint_map));
    memset(__afl_dtaint_map, 0, sizeof(struct dtaint_map));
  }

}

__attribute__((destructor)) 
void __dfsan_hook_debug_fini() {

  fprintf(stderr, "__dfsan_hook_debug_fini\n");
  if (__afl_dtaint_map)
  __dfsan_hook_debug_output();

  if(__afl_dtaint_map) 
    free(__afl_dtaint_map);

}

#endif