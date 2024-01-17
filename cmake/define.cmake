set(PREFIX                          "rowen")

############## OUTPUT DIRECTORIES ###############
set(CMAKE_INSTALL_PREFIX            ${CMAKE_SOURCE_DIR}/install/${PREFIX}-sdk)
set(INSTALL_ROOT_DIRECTORY          "/usr")
set(INSTALL_EXECUTE_DIR             ${CMAKE_INSTALL_PREFIX}/bin)
set(INSTALL_LIBRARY_DIR             ${CMAKE_INSTALL_PREFIX}/lib)
set(INSTALL_INCLUDE_DIR             ${CMAKE_INSTALL_PREFIX}/include)

############## sdk ###########################################
set(SDK_INSTALL_INCLUDE_DIR         ${INSTALL_INCLUDE_DIR}/${PREFIX})

############## cuda ##########################################
set(ROWEN_CUDA                     ${PREFIX}_cuda)
