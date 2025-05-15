# Deploy RSDK package for runtime

# Warn if build type is Debug – deployment is not recommended in this case
if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    message(WARNING "Deploy is recommended for Release or distribution builds")
endif()


# Check if the target path is relative directory, 
if (NOT IS_ABSOLUTE "${DEPLOY_PACKAGE_PATH}")
    set(backup_path ${DEPLOY_PACKAGE_PATH})
    set(DEPLOY_PACKAGE_PATH ${BASE_DIRECTORY}/${DEPLOY_PACKAGE_PATH})
    message(STATUS "Deploy package changed : ${backup_path} -> ${DEPLOY_PACKAGE_PATH}")
endif()


# Logging
message(STATUS "----------------------------------------------------------------")
message(STATUS "  TARGET ROOTFS     : ${CMAKE_SYSROOT}")
message(STATUS "  SOURCE DIRECTORY  : ${CMAKE_INSTALL_PREFIX}")
message(STATUS "  TARGET DIRECTORY  : ${DEPLOY_PACKAGE_PATH}")
message(STATUS "----------------------------------------------------------------")


# Remove existing deploy package if it doesn't start with a protected path
set(PROTECTED_PATH_PREFIXES "/boot" "/etc" "/bin" "/sbin" "/lib" "/lib64" "/usr" "/opt" "/root" "/sys" "/proc")

if (EXISTS "${DEPLOY_PACKAGE_PATH}")
    set(IS_PROTECTED_PATH FALSE)
    foreach(PREFIX IN LISTS PROTECTED_PATH_PREFIXES)
        if (DEPLOY_PACKAGE_PATH MATCHES "^${PREFIX}")
            set(IS_PROTECTED_PATH TRUE)
            break()
        endif()
    endforeach()

    if (NOT IS_PROTECTED_PATH)
        message(STATUS "  Removing existing deploy package: ${DEPLOY_PACKAGE_PATH}")
        file(REMOVE_RECURSE ${DEPLOY_PACKAGE_PATH})
    endif()
endif()


# Deploy package list
file(MAKE_DIRECTORY ${DEPLOY_PACKAGE_PATH})

execute_process(COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_INSTALL_PREFIX}/bin ${DEPLOY_PACKAGE_PATH}/bin)
execute_process(COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_INSTALL_PREFIX}/lib ${DEPLOY_PACKAGE_PATH}/lib)
execute_process(COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_INSTALL_PREFIX}/install.sh ${DEPLOY_PACKAGE_PATH}/install.sh)
execute_process(COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_INSTALL_PREFIX}/uninstall.sh ${DEPLOY_PACKAGE_PATH}/uninstall.sh)
# execute_process(COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_INSTALL_PREFIX}/include ${DEPLOY_PACKAGE_PATH}/include)
# execute_process(COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_INSTALL_PREFIX}/share ${DEPLOY_PACKAGE_PATH}/share)
