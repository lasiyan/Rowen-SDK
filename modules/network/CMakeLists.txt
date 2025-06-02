rs_add_library(
    TYPE    SHARED
    OUTPUT  TARGET
    SOURCES
        src/config_query.cpp
        src/listener_stream.cpp
        src/listener_dgram.cpp
        src/connector_stream.cpp
        src/connector_dgram.cpp
    OUTPUT TARGET
    LINK_LIBS
        PUBLIC
            pthread
            ${PROJECT_NAME}_core
)
