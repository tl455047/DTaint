#include "../dfsan_rt/sanitizer_common/sanitizer_common.h"
#include "../dfsan_rt/sanitizer_common/sanitizer_internal_defs.h"
#include "../dfsan_rt/sanitizer_common/sanitizer_linux.h"

#include "../dfsan_rt/dfsan/dfsan.h"
//#include "../dfsan_rt/include/sanitizer/dfsan_interface.h"
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>

using namespace __dfsan;

#define TAINT_INPUT_LABEL 0xFF

extern "C" {
/**
 * Testing Function.
 */
__attribute__((constructor(1), used)) void __dfsw_debug_func() {
    fprintf(stderr, "dfsan debug.\n");
}

SANITIZER_INTERFACE_ATTRIBUTE 
size_t __dfsw_fread(void *ptr, size_t size, size_t count, FILE *stream,
            dfsan_label ptr_label, dfsan_label size_label,
            dfsan_label count_label, dfsan_label stream_label,
            dfsan_label *ret_label) {
    size_t len = fread(ptr, size, count, stream);
    dfsan_label label; 
    if(len > 0) {
        for(int i = 0; i < len; i++) {
            label = dfsan_create_label(i);
            dfsan_set_label(label, (char* )ptr + i, 1);
        }    
    }
    *ret_label = 0;
    return len;
}

SANITIZER_INTERFACE_ATTRIBUTE 
size_t __dfsw_fread_unlocked(void *ptr, size_t size, size_t n, FILE *stream,
            dfsan_label ptr_label, dfsan_label size_label,
            dfsan_label n_label, dfsan_label stream_label,
            dfsan_label *ret_label) {
    size_t len = fread_unlocked(ptr, size, n, stream);
    if(len > 0)
        dfsan_set_label(TAINT_INPUT_LABEL, ptr, len * size);
    *ret_label = 0;  
    return len;   
}

//dfsan_custom defined
SANITIZER_INTERFACE_ATTRIBUTE
char *__dfsw_fgets(char *s, int size, FILE *stream, dfsan_label s_label,
            dfsan_label size_label, dfsan_label stream_label,
            dfsan_label *ret_label) {
    char *ret = fgets(s, size, stream);
    if (ret) {
        dfsan_set_label(TAINT_INPUT_LABEL, ret, strlen(ret) + 1);
        *ret_label = TAINT_INPUT_LABEL;
    } else {
        *ret_label = 0;
    }
    return ret;
}
//dfsan_custom defined
SANITIZER_INTERFACE_ATTRIBUTE 
ssize_t __dfsw_pread(int fd, void *buf, size_t count, off_t offset,
            dfsan_label fd_label, dfsan_label buf_label,
            dfsan_label count_label, dfsan_label offset_label,
            dfsan_label *ret_label) {
    ssize_t ret = pread(fd, buf, count, offset);
    if (ret > 0)
        dfsan_set_label(TAINT_INPUT_LABEL, buf, ret);
    *ret_label = 0;
    return ret;
}
//dfsan_custom defined
SANITIZER_INTERFACE_ATTRIBUTE 
ssize_t __dfsw_read(int fd, void *buf, size_t count,
            dfsan_label fd_label, dfsan_label buf_label,
            dfsan_label count_label,
            dfsan_label *ret_label) {
    ssize_t ret = read(fd, buf, count);
    if (ret > 0)
        dfsan_set_label(TAINT_INPUT_LABEL, buf, ret);
    *ret_label = 0;
    return ret;
}

SANITIZER_INTERFACE_ATTRIBUTE 
int __dfsw_fgetc(FILE *stream, dfsan_label stream_label, dfsan_label *ret_label) {
    int c = fgetc(stream);
    if(c != EOF) 
        *ret_label = TAINT_INPUT_LABEL;
    else
        *ret_label = 0;
    return c;
}

SANITIZER_INTERFACE_ATTRIBUTE 
int __dfsw_fgetc_unlocked(FILE *stream, dfsan_label stream_label, dfsan_label *ret_label) {
    int c = fgetc_unlocked(stream);
    if(c != EOF) 
        *ret_label = TAINT_INPUT_LABEL;
    else
        *ret_label = 0;
    return c;
}

SANITIZER_INTERFACE_ATTRIBUTE
int __dfsw_getchar(dfsan_label *ret_label) {
    int c = getchar();
    if(c != EOF) 
        *ret_label = TAINT_INPUT_LABEL;
    else
        *ret_label = 0;
    return c;
}

/*SANITIZER_INTERFACE_ATTRIBUTE
char* __dfsw_gets(char* str, dfsan_label str_label, dfsan_label *ret_label) {
    char *ret = gets(str);
  if (ret) {
    dfsan_set_label(TAINT_INPUT_LABEL, ret, strlen(ret) + 1);
    *ret_label = TAINT_INPUT_LABEL;
  } else {
    *ret_label = 0;
  }
  return ret;
}*/

//ssize_t getline(char **restrict lineptr, size_t *restrict n, FILE *restrict stream) 
//ssize_t getdelim(char **restrict lineptr, size_t *restrict n, int delim, FILE *restrict stream)

}
