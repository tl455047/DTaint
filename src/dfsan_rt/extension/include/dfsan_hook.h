#ifndef _DFSAN_HOOK_H_
#define _DFSAN_HOOK_H_

#include "dfsan.h"
#include "dfsan_union_t.h"
void dtaint_dump(dfsan_label_info *label_info);
void dtaint_set_shm(dfsan_label* addr, dfsan_label label);
#endif