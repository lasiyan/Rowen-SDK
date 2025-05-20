rs_add_library(
    SOURCES
        src/queue.cpp
    OUTPUT TARGET
)

target_link_libraries(${TARGET}
    PUBLIC
        pthread
)
