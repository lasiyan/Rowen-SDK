add_subdirectory(core)
add_subdirectory(logger)
add_subdirectory(network)
add_subdirectory(ipc)
add_subdirectory(utils)

if (OPTION_WITH_RSDK_VISION)
    add_subdirectory(vision)
endif()
