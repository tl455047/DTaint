set(CMAKE_CXX_STANDARD 11)

add_library(memlog_rt STATIC
            memlog.c)

include_directories(include)
include_directories(../afl_rt/include)

set_target_properties(memlog_rt 
    PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY "${DTAINT_LIB_DIR}"
    COMPILE_FLAGS "-funroll-loops -fPIC -fPIE -O3 -Wall -D_FORTIFY_SOURCE=2 -g -Wno-pointer-sign")

option(MEMLOG_DEBUG "Memlog Debugging" OFF)

if(MEMLOG_DEBUG)
    target_compile_options(memlog_rt PRIVATE -DMEMLOG_DEBUG)
endif()