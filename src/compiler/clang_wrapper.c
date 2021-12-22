/**
 * Modified from AFL afl-clang-fast.c and Angora angora-clang.c,
 * as a wrapper for clang. 
 */
#define AFL_MAIN

#include "include/config.h"
#include "include/types.h"
#include "include/debug.h"
#include "include/alloc-inl.h"

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

static u8*  obj_path;               /* Path to runtime libraries         */
static u8** cc_params;              /* Parameters passed to the real CC  */
static u32  cc_par_cnt = 1;         /* Param count, including argv0      */

/* Try to find the runtime libraries. If that fails, abort. */

static void find_obj(u8* argv0) {

  u8 *slash, *tmp;

  slash = strrchr(argv0, '/');

  if (slash) {

    u8 *dir;

    *slash = 0;
    dir = ck_strdup(argv0);
    *slash = '/';

    tmp = alloc_printf("%s/pass/libDFSanPass.so", dir);

    if (!access(tmp, R_OK)) {
      obj_path = dir;
      ck_free(tmp);
      return;
    }

    ck_free(tmp);
    ck_free(dir);

  }

  FATAL("Unable to find 'afl-llvm-rt.o' or 'afl-llvm-pass.so'. Please set AFL_PATH");

}

static void memlog_pass() {
  
  /**
   * Need to enable corresponding llvm optimization level, 
   * where your pass is registed.
   */
  cc_params[cc_par_cnt++] = "-Xclang";
  cc_params[cc_par_cnt++] = "-load";
  cc_params[cc_par_cnt++] = "-Xclang";
  cc_params[cc_par_cnt++] = alloc_printf("%s/pass/libMemlogPass.so", obj_path);

  cc_params[cc_par_cnt++] = "-mllvm";
  cc_params[cc_par_cnt++] = "-memlog-hook-inst=1";
       
  cc_params[cc_par_cnt++] = "-mllvm";
  cc_params[cc_par_cnt++] = 
        alloc_printf("-memlog-hook-abilist=%s/lib/share/hook_abilist.txt", obj_path);
  
  cc_params[cc_par_cnt++] = "-mllvm";
  cc_params[cc_par_cnt++] = 
        alloc_printf("-memlog-debug=0", obj_path);

}

static void dfsan_pass() {

  /**
   * Need to enable corresponding llvm optimization level, 
   * where your pass is registed.
   */
  cc_params[cc_par_cnt++] = "-Xclang";
  cc_params[cc_par_cnt++] = "-load";
  cc_params[cc_par_cnt++] = "-Xclang";
  cc_params[cc_par_cnt++] = alloc_printf("%s/pass/libDFSanPass.so", obj_path);
  
  cc_params[cc_par_cnt++] = "-mllvm";
  cc_params[cc_par_cnt++] =  
        alloc_printf("-dtaint-dfsan-abilist=%s/lib/share/dfsan_abilist.txt", obj_path);

  cc_params[cc_par_cnt++] = "-mllvm";
  cc_params[cc_par_cnt++] =  
        alloc_printf("-dtaint-dfsan-abilist=%s/lib/share/target_abilist.txt", obj_path);
  
  /*cc_params[cc_par_cnt++] = "-mllvm";
  cc_params[cc_par_cnt++] =  
        alloc_printf("-dtaint-dfsan-abilist=%s/lib/share/libexif_abilist.txt", obj_path);*/
  /**
   * After llvm-10, DFSan supports callback for load, store, mem transfer, and cmp.
   * Enable with argument -taint-dfsan-event-callbacks. 
   */
  //cc_params[cc_par_cnt++] = "-mllvm";
  //cc_params[cc_par_cnt++] = "-taint-dfsan-event-callbacks=1";
  /*cc_params[cc_par_cnt++] = "-mllvm";
  cc_params[cc_par_cnt++] =  
        alloc_printf("-dtaint-dfsan-abilist=%s/lib/share/openjpeg_abilist.txt", obj_path);*/

  cc_params[cc_par_cnt++] = "-mllvm";
  cc_params[cc_par_cnt++] = "-dtaint-dfsan-combine-pointer-labels-on-store=1";

  cc_params[cc_par_cnt++] = "-mllvm";
  cc_params[cc_par_cnt++] = "-dtaint-dfsan-hook-inst=1";

  cc_params[cc_par_cnt++] = "-mllvm";
  cc_params[cc_par_cnt++] = 
        alloc_printf("-dtaint-dfsan-hook-abilist=%s/lib/share/hook_abilist.txt", obj_path);

  cc_params[cc_par_cnt++] = "-mllvm";
  cc_params[cc_par_cnt++] = 
        alloc_printf("-dtaint-dfsan-hook-debug=1", obj_path);

  cc_params[cc_par_cnt++] = "-mllvm";
  cc_params[cc_par_cnt++] =
        alloc_printf("-dtaint-dfsan-debug-nonzero-labels=0", obj_path);

  cc_params[cc_par_cnt++] = "-mllvm";
  cc_params[cc_par_cnt++] = 
        alloc_printf("-dtaint-dfsan-combine-pointer-labels-on-load=1", obj_path);

  cc_params[cc_par_cnt++] = "-mllvm";
  cc_params[cc_par_cnt++] = 
        alloc_printf("-dtaint-dfsan-event-callbacks=0", obj_path);
}

static void memlog_runtime() {
    //TO DO
    /**
     * --whole-archive will force compiler to link static library, even if
     * function in library is not used. This is useful for regist function
     * into __libc_csu_init.
     */
    cc_params[cc_par_cnt++] = "-Wl,--whole-archive";
    cc_params[cc_par_cnt++] = alloc_printf("%s/lib/libmemlog_rt.a", obj_path);
    cc_params[cc_par_cnt++] = "-Wl,--no-whole-archive";
}

static void dfsan_runtime() {
  cc_params[cc_par_cnt++] = "-Wl,--whole-archive";
  cc_params[cc_par_cnt++] = alloc_printf("%s/lib/libclang_rt.dfsan-x86_64.a", obj_path);
  cc_params[cc_par_cnt++] = "-Wl,--no-whole-archive";
  cc_params[cc_par_cnt++] =
        alloc_printf("-Wl,--dynamic-list=%s/lib/libclang_rt.dfsan-x86_64.a.syms", obj_path);

  cc_params[cc_par_cnt++] = "-ldl";
  cc_params[cc_par_cnt++] = "-lpthread";    
  cc_params[cc_par_cnt++] = "-lstdc++";
}

static void afl_runtime() {
  cc_params[cc_par_cnt++] = "-lrt";
  cc_params[cc_par_cnt++] = "-Wl,--whole-archive";
  cc_params[cc_par_cnt++] = alloc_printf("%s/lib/libafl_rt.a", obj_path);
  cc_params[cc_par_cnt++] = "-Wl,--no-whole-archive";
}

static void link_constructor() {
  /**
   * Link __attribute__((constructor)) function, seems that in static library,
   * linker will not link the function if the function is not used in program.
   * Maybe can use __attribute__((used)) ?
   */ 
  //cc_params[cc_par_cnt++] = "-Wl,-u,__dfsw_debug_func";
  
}


static void sync_hook_id(char *dst, char *src) {
  // In order to make sure MemlogPass and DFSanPass hook same instructions 
  // with same HookID.
  char buf[8];
  memset(buf, 0, 8);
  FILE* src_f = fopen(src, "r");
  fread(buf, 1, sizeof(unsigned int), src_f);
  fclose(src_f);

  FILE* dst_f = fopen(dst, "w+");
  fwrite(buf, 1, sizeof(unsigned int), dst_f);
  fclose(dst_f);
}
/* Copy argv to cc_params, making the necessary edits. */

static void edit_params(u32 argc, char** argv) {

  u8 fortify_set = 0, asan_set = 0, x_set = 0, bit_mode = 0;
  u8 *name;

  cc_params = ck_alloc((argc + 128) * sizeof(u8*));

  name = strrchr(argv[0], '/');
  if (!name) name = argv[0]; else name++;

  if (!strncmp(name, "clang-wrapper++", 15)) {
    u8* alt_cxx = getenv("TAINT_CXX");
    cc_params[0] = alt_cxx ? alt_cxx : (u8*)"clang++";
  } else {
    u8* alt_cc = getenv("TAINT_CC");
    cc_params[0] = alt_cc ? alt_cc : (u8*)"clang";
  }

  /* There are two ways to compile afl-clang-fast. In the traditional mode, we
     use afl-llvm-pass.so to inject instrumentation. In the experimental
     'trace-pc-guard' mode, we use native LLVM instrumentation callbacks
     instead. The latter is a very recent addition - see:

     http://clang.llvm.org/docs/SanitizerCoverage.html#tracing-pcs-with-guards */

/*#ifdef USE_TRACE_PC
  cc_params[cc_par_cnt++] = "-fsanitize-coverage=trace-pc-guard";
#ifndef __ANDROID__
  cc_params[cc_par_cnt++] = "-mllvm";
  cc_params[cc_par_cnt++] = "-sanitizer-coverage-block-threshold=0";
#endif
#else
  cc_params[cc_par_cnt++] = "-Xclang";
  cc_params[cc_par_cnt++] = "-load";
  cc_params[cc_par_cnt++] = "-Xclang";
  cc_params[cc_par_cnt++] = alloc_printf("%s/afl-llvm-pass.so", obj_path);
#endif*/ /* ^USE_TRACE_PC */

  cc_params[cc_par_cnt++] = "-Qunused-arguments";

  while (--argc) {
    u8* cur = *(++argv);

    if (!strcmp(cur, "-m32")) bit_mode = 32;
    if (!strcmp(cur, "armv7a-linux-androideabi")) bit_mode = 32;
    if (!strcmp(cur, "-m64")) bit_mode = 64;

    if (!strcmp(cur, "-x")) x_set = 1;

    if (!strcmp(cur, "-fsanitize=address") ||
        !strcmp(cur, "-fsanitize=memory")) asan_set = 1;

    if (strstr(cur, "FORTIFY_SOURCE")) fortify_set = 1;

    if (!strcmp(cur, "-Wl,-z,defs") ||
        !strcmp(cur, "-Wl,--no-undefined")) continue;

    cc_params[cc_par_cnt++] = cur;

  }

  if (getenv("TAINT_HARDEN")) {

    cc_params[cc_par_cnt++] = "-fstack-protector-all";

    if (!fortify_set)
      cc_params[cc_par_cnt++] = "-D_FORTIFY_SOURCE=2";

  }

  if (!asan_set) {

    if (getenv("TAINT_USE_ASAN")) {

      if (getenv("TAINT_USE_MSAN"))
        FATAL("ASAN and MSAN are mutually exclusive");

      if (getenv("TAINT_HARDEN"))
        FATAL("ASAN and TAINT_HARDEN are mutually exclusive");

      cc_params[cc_par_cnt++] = "-U_FORTIFY_SOURCE";
      cc_params[cc_par_cnt++] = "-fsanitize=address";

    } else if (getenv("TAINT_USE_MSAN")) {

      if (getenv("TAINT_USE_ASAN"))
        FATAL("ASAN and MSAN are mutually exclusive");

      if (getenv("TAINT_HARDEN"))
        FATAL("MSAN and TAINT_HARDEN are mutually exclusive");

      cc_params[cc_par_cnt++] = "-U_FORTIFY_SOURCE";
      cc_params[cc_par_cnt++] = "-fsanitize=memory";

    }

  }

#ifdef USE_TRACE_PC

  if (getenv("TAINT_INST_RATIO"))
    FATAL("TAINT_INST_RATIO not available at compile time with 'trace-pc'.");

#endif /* USE_TRACE_PC */

   /**
   * Turn off builtin functions, otherwise llvm will treat these functions
   * as llvm intrinsic, DFSan does not support instrumentation for intrinsic
   * functions.
   * Not always useful.
   * 
   * memset in llvm always be transformed as llvm.memset.*, will this
   * affect taint analysis ?
   */
  //cc_params[cc_par_cnt++] = "-ffreestanding";
  cc_params[cc_par_cnt++] = "-fno-builtin-memcpy";
  //cc_params[cc_par_cnt++] = "-fno-builtin-memset";
  //cc_params[cc_par_cnt++] = "-fno-builtin-memmove";

  if (!getenv("TAINT_DONT_OPTIMIZE")) {

    cc_params[cc_par_cnt++] = "-g";
    cc_params[cc_par_cnt++] = "-O3";
    cc_params[cc_par_cnt++] = "-funroll-loops";
  }
  
  if (getenv("DTAINT_MODE")) {
    
    dfsan_pass();
    dfsan_runtime();

  }
  else {
    
    memlog_pass(); 
    memlog_runtime();
  
  }

  afl_runtime();
  
  /*if (getenv("SYNC_HOOK_ID")) {
    fprintf(stderr, "sync_hook_id\n");
    if (getenv("MEMLOG_MODE")) {
      sync_hook_id("/tmp/.DtaintHookID.txt", "/tmp/.MemlogHookID.txt");
    }
    else {
      sync_hook_id("/tmp/.MemlogHookID.txt", "/tmp/.DtaintHookID.txt");
    }
    unsetenv("SYNC_HOOK_ID");
  }*/
  
  /**
   * Enable pie since dfsan maps shadow memory at 0x10000-0x200200000000, 
   * pie is needed to prevent overlapped.
   */
  cc_params[cc_par_cnt++] = "-fPIE";
  cc_params[cc_par_cnt++] = "-fPIC";
  cc_params[cc_par_cnt++] = "-pie";
  
  /*if (getenv("TAINT_NO_BUILTIN")) {

    cc_params[cc_par_cnt++] = "-fno-builtin-strcmp";
    cc_params[cc_par_cnt++] = "-fno-builtin-strncmp";
    cc_params[cc_par_cnt++] = "-fno-builtin-strcasecmp";
    cc_params[cc_par_cnt++] = "-fno-builtin-strncasecmp";
    cc_params[cc_par_cnt++] = "-fno-builtin-memcmp";

  }*/

  /*cc_params[cc_par_cnt++] = "-D__AFL_HAVE_MANUAL_CONTROL=1";
  cc_params[cc_par_cnt++] = "-D__AFL_COMPILER=1";
  cc_params[cc_par_cnt++] = "-DFUZZING_BUILD_MODE_UNSAFE_FOR_PRODUCTION=1";*/

  /* When the user tries to use persistent or deferred forkserver modes by
     appending a single line to the program, we want to reliably inject a
     signature into the binary (to be picked up by afl-fuzz) and we want
     to call a function from the runtime .o file. This is unnecessarily
     painful for three reasons:

     1) We need to convince the compiler not to optimize out the signature.
        This is done with __attribute__((used)).

     2) We need to convince the linker, when called with -Wl,--gc-sections,
        not to do the same. This is done by forcing an assignment to a
        'volatile' pointer.

     3) We need to declare __afl_persistent_loop() in the global namespace,
        but doing this within a method in a class is hard - :: and extern "C"
        are forbidden and __attribute__((alias(...))) doesn't work. Hence the
        __asm__ aliasing trick.

   */

 /* cc_params[cc_par_cnt++] = "-D__AFL_LOOP(_A)="
    "({ static volatile char *_B __attribute__((used)); "
    " _B = (char*)\"" PERSIST_SIG "\"; "
#ifdef __APPLE__
    "__attribute__((visibility(\"default\"))) "
    "int _L(unsigned int) __asm__(\"___afl_persistent_loop\"); "
#else
    "__attribute__((visibility(\"default\"))) "
    "int _L(unsigned int) __asm__(\"__afl_persistent_loop\"); "
#endif*/ /* ^__APPLE__ */
    /*"_L(_A); })";

  cc_params[cc_par_cnt++] = "-D__AFL_INIT()="
    "do { static volatile char *_A __attribute__((used)); "
    " _A = (char*)\"" DEFER_SIG "\"; "
#ifdef __APPLE__
    "__attribute__((visibility(\"default\"))) "
    "void _I(void) __asm__(\"___afl_manual_init\"); "
#else
    "__attribute__((visibility(\"default\"))) "
    "void _I(void) __asm__(\"__afl_manual_init\"); "
#endif*/ /* ^__APPLE__ */
   // "_I(); } while (0)";

  if (x_set) {
    cc_params[cc_par_cnt++] = "-x";
    cc_params[cc_par_cnt++] = "none";
  }

/*#ifndef __ANDROID__
  switch (bit_mode) {

    case 0:
      cc_params[cc_par_cnt++] = alloc_printf("%s/afl-llvm-rt.o", obj_path);
      break;

    case 32:
      cc_params[cc_par_cnt++] = alloc_printf("%s/afl-llvm-rt-32.o", obj_path);

      if (access(cc_params[cc_par_cnt - 1], R_OK))
        FATAL("-m32 is not supported by your compiler");

      break;

    case 64:
      cc_params[cc_par_cnt++] = alloc_printf("%s/afl-llvm-rt-64.o", obj_path);

      if (access(cc_params[cc_par_cnt - 1], R_OK))
        FATAL("-m64 is not supported by your compiler");

      break;

  }
#endif*/

  cc_params[cc_par_cnt] = NULL;

}

static void print_cmdline(int argc) {

  for(int i = 0; i < argc + 128; i++) {
    if(cc_params[i] == NULL)
      break;
    printf("%s ", cc_params[i]);
  }printf("\n");
}
/* Main entry point */

int main(int argc, char** argv) {

  if (isatty(2) && !getenv("AFL_QUIET")) {

/*#ifdef USE_TRACE_PC
    SAYF(cCYA "afl-clang-fast [tpcg] " cBRI VERSION  cRST " by <lszekeres@google.com>\n");
#else
    SAYF(cCYA "afl-clang-fast " cBRI VERSION  cRST " by <lszekeres@google.com>\n");
#endif*/ /* ^USE_TRACE_PC */

  }

  if (argc < 2) {

    SAYF("\n"
         "This is a helper application for afl-fuzz. It serves as a drop-in replacement\n"
         "for clang, letting you recompile third-party code with the required runtime\n"
         "instrumentation. A common use pattern would be one of the following:\n\n"

         "  CC=path/clang-wrapper ./configure\n"
         "  CXX=path/clang-wrapper++ ./configure\n\n"

         "In contrast to the traditional afl-clang tool, this version is implemented as\n"
         "an LLVM pass and tends to offer improved performance with slow programs.\n\n"

         "You can specify custom next-stage toolchain via AFL_CC and AFL_CXX. Setting\n"
         "AFL_HARDEN enables hardening optimizations in the compiled code.\n\n");

    exit(1);

  }


#ifndef __ANDROID__
  find_obj(argv[0]);
#endif

  edit_params(argc, argv);

  print_cmdline(argc);

  
  execvp(cc_params[0], (char**)cc_params);
 
  FATAL("Oops, failed to execute '%s' - check your PATH", cc_params[0]);

  return 0;

}
