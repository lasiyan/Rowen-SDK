rs_add_library(
    TYPE    SHARED
    OUTPUT  TARGET
    SOURCES
        src/file.cpp
        src/resource.cpp
        src/threadPool.cpp
        src/twilight.cpp
        src/table.cpp
        src/tableExt.cpp
    OUTPUT TARGET
    LINK_LIBS
        PUBLIC
            pthread
            ${PROJECT_NAME}_core
)
