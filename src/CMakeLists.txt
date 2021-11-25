cmake_minimum_required(VERSION 3.1)

include_directories(dfsan_rt)

set(DTAINT_BIN_DIR "${CMAKE_BINARY_DIR}")
set(DTAINT_LIB_DIR "${CMAKE_BINARY_DIR}/lib")
set(DTAINT_PASS_DIR "${CMAKE_BINARY_DIR}/pass")

add_subdirectory(abilist)
add_subdirectory(afl_rt)
add_subdirectory(compiler)
add_subdirectory(dfsan_rt)
add_subdirectory(pass)
add_subdirectory(memlog_rt)

#dfsan_abilist
set(dfsan_abilist_dir ${DTAINT_LIB_DIR}/share)
set(dfsan_abilist_filename ${dfsan_abilist_dir}/dfsan_abilist.txt)
add_custom_target(dfsan_abilist ALL
  DEPENDS ${dfsan_abilist_filename})
add_custom_command(OUTPUT ${dfsan_abilist_filename}
                   VERBATIM
                   COMMAND
                    ${CMAKE_COMMAND} -E make_directory ${dfsan_abilist_dir}
                   COMMAND
                     cat ${CMAKE_CURRENT_SOURCE_DIR}/abilist/done_abilist.txt
                         ${CMAKE_CURRENT_SOURCE_DIR}/abilist/libc_ubuntu1404_abilist.txt
                         > ${dfsan_abilist_filename}
                   DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/abilist/done_abilist.txt
                           ${CMAKE_CURRENT_SOURCE_DIR}/abilist/libc_ubuntu1404_abilist.txt)
add_dependencies(dfsan dfsan_abilist)
install(FILES ${dfsan_abilist_filename}
        DESTINATION ${DTAINT_LIB_DIR}/share)
#hook_abilist
set(hook_abilist_dir ${DTAINT_LIB_DIR}/share)
set(hook_abilist_filename ${hook_abilist_dir}/hook_abilist.txt)
add_custom_target(hook_abilist ALL
  DEPENDS ${hook_abilist_filename})
add_custom_command(OUTPUT ${hook_abilist_filename}
                   VERBATIM
                   COMMAND
                    ${CMAKE_COMMAND} -E make_directory ${hook_abilist_dir}
                   COMMAND
                     cat ${CMAKE_CURRENT_SOURCE_DIR}/abilist/hook_abilist.txt
                         > ${hook_abilist_filename}
                   DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/abilist/hook_abilist.txt)
install(FILES ${hook_abilist_filename}
        DESTINATION ${DTAINT_LIB_DIR}/share)