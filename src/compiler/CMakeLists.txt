
add_executable(clang-wrapper clang_wrapper.c)

include_directories(include)

set_target_properties(clang-wrapper
    PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${TAINT_BIN_DIR}"
)

add_custom_command(TARGET clang-wrapper POST_BUILD
    COMMAND ln -sf "clang-wrapper" "${TAINT_BIN_DIR}/clang-wrapper++")
