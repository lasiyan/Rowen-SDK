rs_add_library(
    SOURCES
        src/file.cpp
        src/resource.cpp
        src/threadPool.cpp
        src/twilight.cpp
        src/table.cpp
        src/tableExt.cpp
    OUTPUT TARGET
)

target_link_libraries(${TARGET}
    PUBLIC
        ${PROJECT_NAME}_core
        pthread
)
