include(${CMAKE_CURRENT_LIST_DIR}/config_allocator.cmake)
# --------------------------------------------------------------------------------
# Search platform information in current system
# And generate rowen/config.h


# Logging
message(STATUS "Target System    : ${CMAKE_SYSTEM_NAME}")
message(STATUS "Target Processor : ${CMAKE_SYSTEM_PROCESSOR}")
message(STATUS "Target Root FS   : ${CMAKE_SYSROOT}")


# -------------------------------- Detection -------------------------------------
include(${CMAKE_CURRENT_LIST_DIR}/config_system_jetpack.cmake)

# --- Linux
if( ${CMAKE_SYSTEM_NAME} STREQUAL "Linux" )
    list(APPEND RS_CONFIG_FLAGS __platform_linux__)

    # --- Linux x86_64
    if( ${CMAKE_SYSTEM_PROCESSOR} STREQUAL "x86_64" )
        list(APPEND RS_CONFIG_FLAGS __platform_x86_64__)

    # --- Linux aarch64
    elseif( ${CMAKE_SYSTEM_PROCESSOR} STREQUAL "aarch64" )
        list(APPEND RS_CONFIG_FLAGS __platform_aarch64__)
        list(APPEND RS_CONFIG_FLAGS __platform_arm64__)

        if (DEVICE_MODEL STREQUAL "") # for Cross compile
            message(WARNING "Device model information is empty ..")
        else()
            message(STATUS "Target Hardware : ${DEVICE_MODEL}")
            STRING(TOLOWER "${DEVICE_MODEL}" DEVICE_MODEL) # To lower case (for easy find)
        endif()

        # --- Find Device Model --------------------------------------------------
        if (EXISTS "/etc/nv_tegra_release" OR DEVICE_MODEL MATCHES "jetson|nvidia")
            # --- Linux aarch64 Jetson
            jetpack_version_configure("/etc/nv_tegra_release" RS_CONFIG_FLAGS)
        elseif (DEVICE_MODEL MATCHES "texas")
            # --- Linux TI board
            list(APPEND RS_CONFIG_FLAGS __platform_TI__)
            list(APPEND RS_CONFIG_FLAGS __platform_texas__)
        endif()

    # --- Linux Unsupported
    else()
        message(FATAL_ERROR "Unsupported Linux platform: ${CMAKE_SYSTEM_PROCESSOR}")
    endif()

# --- Windows
elseif( ${CMAKE_SYSTEM_NAME} STREQUAL "Windows" )
    list(APPEND RS_CONFIG_FLAGS __platform_windows__)

    # --- Windows x86_64
    if( ${CMAKE_SYSTEM_PROCESSOR} STREQUAL "x86_64" )
        list(APPEND RS_CONFIG_FLAGS __platform_x86_64__)

    # --- Windows Unsupported
    else()
        message(FATAL_ERROR "Unsupported Windows platform: ${CMAKE_SYSTEM_PROCESSOR}")
    endif()
    
# --- Mac OS
elseif( ${CMAKE_SYSTEM_NAME} STREQUAL "Darwin" )
    list(APPEND RS_CONFIG_FLAGS __platform_macos__)

    # --- Mac OS x86_64
    if( ${CMAKE_SYSTEM_PROCESSOR} STREQUAL "x86_64" )
        list(APPEND RS_CONFIG_FLAGS __platform_x86_64__)
    
    # --- Mac OS arm64
    elseif( ${CMAKE_SYSTEM_PROCESSOR} STREQUAL "arm64" )
        list(APPEND RS_CONFIG_FLAGS __platform_aarch64__)
        list(APPEND RS_CONFIG_FLAGS __platform_arm64__)
    
    # --- Mac OS Unsupported
    else()
        message(FATAL_ERROR "Unsupported macOS platform: ${CMAKE_SYSTEM_PROCESSOR}")
    endif()
    
# --- Unsupported
else()
    message(FATAL_ERROR "Unsupported OS: ${CMAKE_SYSTEM_NAME}")
endif()

# --------------------------------------------------------------------------------
