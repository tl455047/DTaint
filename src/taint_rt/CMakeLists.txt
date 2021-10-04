set(CMAKE_CXX_STANDARD 11)

add_library(taint_rt STATIC
            taint.c)

include_directories(include)

set_target_properties(taint_rt 
    PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY "${TAINT_LIB_DIR}"
    COMPILE_FLAGS "-funroll-loops -fPIC -fPIE -O3 -Wall -D_FORTIFY_SOURCE=2 -g -Wno-pointer-sign"
)
