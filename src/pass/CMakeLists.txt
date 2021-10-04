
find_package(LLVM REQUIRED CONFIG)

if (LLVM_FOUND)
    message(STATUS "LLVM_VERSION_MAJOR: ${LLVM_VERSION_MAJOR}")
    message(STATUS "LLVM_VERSION_MINOR: ${LLVM_VERSION_MINOR}")
    message(STATUS "LLVM_VERSION_PATCH: ${LLVM_VERSION_PATCH}")
    if (${LLVM_VERSION_MAJOR} GREATER 11 OR ${LLVM_VERSION_MAJOR} LESS 11)
        message(FATAL_ERROR "LLVM version should be 11.0.0 !")
    endif()
else()
    message(FATAL_ERROR "You haven't install LLVM !")
endif()

if (NOT TARGET LLVMPassConfig)
    add_library(LLVMPassConfig INTERFACE IMPORTED)
    set_target_properties(LLVMPassConfig PROPERTIES
        INTERFACE_COMPILE_OPTIONS "-fno-rtti" #-fpic
        INTERFACE_INCLUDE_DIRECTORIES "${LLVM_INCLUDE_DIRS}"
        INTERFACE_LINK_DIRECTORIES "${LLVM_LIBRARY_DIRS}"
        INTERFACE_COMPILE_DEFINITIONS "LLVM_VERSION_MAJOR=${LLVM_VERSION_MAJOR};LLVM_VERSION_MINOR=${LLVM_VERSION_MINOR};MAP_SIZE_POW2=${MAP_SIZE_POW2}"
        # INTERFACE_LINK_OPTIONS "-Wl,-znodelete"
    )
endif()

# fix pass bug: https://github.com/sampsyo/llvm-pass-skeleton/issues/7#issuecomment-401834287
set (CMAKE_MODULE_LINKER_FLAGS "${CMAKE_CXX_LINK_FLAGS} -Wl,-znodelete")

#DataFlowSanitizer Pass
add_library(DFSanPass MODULE DataFlowSanitizer.cpp)
target_link_libraries(DFSanPass LLVMPassConfig)
#install (TARGETS DFSanPass DESTINATION ${TAINT_PASS_DIR})
#Taint Pass
add_library(TaintPass MODULE TaintPass.cc)
target_link_libraries(TaintPass LLVMPassConfig)
#install (TARGETS TaintPass DESTINATION ${TAINT_PASS_DIR})

set_target_properties(DFSanPass
                      TaintPass
    PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY "${TAINT_PASS_DIR}"
)
