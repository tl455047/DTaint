#include <stdio.h>
#include "dfsan.h"
#include "sanitizer_common/sanitizer_common.h"

static int total_count = 0, taint_count = 0, total_load_count = 0, taint_load_count = 0;

extern "C" {
void __dfsan_store_callback(dfsan_label Label) {
  if (!Label)
    return;
  //fprintf(stderr, "Label %u stored to memory\n", Label);
}

void __dfsan_load_callback(dfsan_label Label) {
  total_load_count++;
  if (!Label)
   return;
  taint_load_count++;
  //fprintf(stderr, "Label %u loaded from memory %d/%d\n", Label, taint_load_count, total_load_count);
  //fprintf(stderr, "Offset %s\n", dfsan_map_get_bitset_string(Label).c_str()); 
}

void __dfsan_mem_transfer_callback(dfsan_label *Start, size_t Len) {

  //fprintf(stderr, "Label %u copied to memory\n", Start[0]);
}

void __dfsan_cmp_callback(dfsan_label CombinedLabel) {
  total_count++;
  if (!CombinedLabel)
    return;
  taint_count++;
  //fprintf(stderr, "Label %u used for branching %d/%d\n", CombinedLabel, taint_count, total_count);

  //  fprintf(stderr, "Offset %s\n", dfsan_map_get_bitset_string(CombinedLabel).c_str()); 
}
}
