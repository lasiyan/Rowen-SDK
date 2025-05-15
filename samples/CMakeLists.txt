# 3rd party
add_subdirectory(example-jsoncpp)
add_subdirectory(example-yaml)

# core
add_subdirectory(example-time)
add_subdirectory(example-response)

# logger
add_subdirectory(example-logger)

# IPC
add_subdirectory(example-domain)

# network
add_subdirectory(example-network)

# utils
add_subdirectory(example-threadpool)
add_subdirectory(example-twilight)
add_subdirectory(example-resource)
add_subdirectory(example-table)

# vision
if (OPTION_WITH_RSDK_VISION)
    add_subdirectory(example-vision)
    add_subdirectory(example-geometry)
    add_subdirectory(example-pango)
endif()
