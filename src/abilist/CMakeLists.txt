#target_abilist
set(target_abilist_dir ${DTAINT_LIB_DIR}/share)
set(target_abilist_filename ${target_abilist_dir}/target_abilist.txt)
add_custom_target(target_abilist ALL
  DEPENDS ${target_abilist_filename})
message(STATUS ${CMAKE_CURRENT_SOURCE_DIR})
add_custom_command(OUTPUT ${target_abilist_filename}
                   VERBATIM
                   COMMAND
                    ${CMAKE_COMMAND} -E make_directory ${target_abilist_dir}
                   COMMAND
                      find ${CMAKE_CURRENT_SOURCE_DIR}/target -type f -exec cat {} + 
                      > ${target_abilist_filename}

                   DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/target/*)
install(FILES ${target_abilist_filename}
        DESTINATION ${DTAINT_LIB_DIR}/share)