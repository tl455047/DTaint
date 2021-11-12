set(CMAKE_CXX_STANDARD 11)

add_library(afl_rt STATIC
            afl-compiler-rt.o.c)

include_directories(include)
include_directories(../dfsan_rt/extension/include)
include_directories(../memlog_rt/include)

set_target_properties(afl_rt 
    PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY "${TAINT_LIB_DIR}"
    COMPILE_FLAGS "-funroll-loops -fPIC -fPIE -O3 -Wall -D_FORTIFY_SOURCE=2 -g -Wno-pointer-sign"
)
