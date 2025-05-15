rs_add_library(
    SOURCES
        src/domain_listener.cpp
        src/domain_connector.cpp
    OUTPUT TARGET
)

target_link_libraries(${TARGET}
    PUBLIC
        rt
        pthread
        ${PROJECT_NAME}_transport
)
