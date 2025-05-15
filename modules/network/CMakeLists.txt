rs_add_library(
    SOURCES
        src/config_query.cpp
        src/listener_stream.cpp
        src/listener_dgram.cpp
        src/connector_stream.cpp
        src/connector_dgram.cpp
    OUTPUT TARGET
)

target_link_libraries(${TARGET}
    PUBLIC
        pthread
        ${PROJECT_NAME}_core
        ${PROJECT_NAME}_transport
)
