#include <stdio.h>
#include "dfsan.h"
#include "dfsan_union_t.h"
#include "sanitizer_common/sanitizer_common.h"

extern struct dfsan_label_info __dfsan_label_info;
//static int total_count = 0, taint_count = 0, total_load_count = 0, taint_load_count = 0;

extern "C" {
void __dfsan_store_callback(dfsan_label Label) {
  if (!Label)
    return;
  //fprintf(stderr, "Label %u stored to memory\n", Label);
}

/**
 * Origin:
 *   void __dfsan_load_callback(dfsan_label Label); 
 * Modified:
 *   void __dfsan_load_callback(dfsan_label Label, void* ptr, size_t size);
 */
/*void __dfsan_load_callback(dfsan_label Label) {
  total_load_count++;
  if (!Label)
   return;
  taint_load_count++;
  //dfsan_union_t_output_offset(&__dfsan_label_info, Label);
  dfsan_hook_load_inst(&__dfsan_label_info, Label);
  //fprintf(stderr, "Label %u loaded from memory %u/%u\n", Label, taint_load_count, total_load_count);
}*/

void __dfsan_load_callback(dfsan_label Label, void* ptr, size_t size) {
  //total_load_count++;
  if (!Label)
   return;
  //taint_load_count++;
  //dfsan_union_t_output_offset(&__dfsan_label_info, Label);
  
}

void __dfsan_mem_transfer_callback(dfsan_label *Start, size_t Len) {

  //fprintf(stderr, "Label %u copied to memory\n", Start[0]);
}

void __dfsan_cmp_callback(dfsan_label CombinedLabel) {
  //total_count++;
  if (!CombinedLabel)
    return;
  //taint_count++;
  //fprintf(stderr, "Label %u used for branching\n", CombinedLabel);
  
}


}