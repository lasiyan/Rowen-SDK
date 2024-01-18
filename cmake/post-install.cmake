# Get install libraries(.so)
# set(INSTALL_LIBRARY_ROOT "/usr")
# set(INSTALL_LIBRARY_LIST "")

# if(NOT EXISTS "${CMAKE_CURRENT_BINARY_DIR}/install_manifest.txt")
#   message(FATAL_ERROR "Cannot find install manifest: ${CMAKE_CURRENT_BINARY_DIR}/install_manifest.txt")
# endif()

# file(READ "${CMAKE_CURRENT_BINARY_DIR}/install_manifest.txt" files)
# string(REGEX REPLACE "\n" ";" files "${files}")
# foreach(file ${files})
#   if(file MATCHES "lib.*\\.so")
#     get_filename_component(file_name "${file}" NAME)
#     list(APPEND INSTALL_LIBRARY_LIST "${file_name}")
#   endif()
# endforeach()

# -----------------------------------------------------------------------------

configure_file(
  "${CMAKE_CURRENT_LIST_DIR}/post-install.sh.in"
  "${CMAKE_INSTALL_PREFIX}/install.sh" IMMEDIATE @ONLY
)
execute_process(COMMAND chmod 777 "${CMAKE_INSTALL_PREFIX}/install.sh")

configure_file(
  "${CMAKE_CURRENT_LIST_DIR}/post-uninstall.sh.in"
  "${CMAKE_INSTALL_PREFIX}/uninstall.sh" IMMEDIATE @ONLY
)
execute_process(COMMAND chmod 777 "${CMAKE_INSTALL_PREFIX}/uninstall.sh")
