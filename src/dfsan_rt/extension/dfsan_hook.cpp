#include "dfsan_hook.h"
#include "dtaint.h"
#include "sanitizer_common/sanitizer_common.h"
#include "types.h" 
#include <cstring>
#include <stdint.h>
#include <stdarg.h>

extern struct dfsan_label_info __dfsan_label_info;
extern struct dtaint_map *__afl_dtaint_map;
