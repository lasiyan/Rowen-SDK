rs_add_library(
    # NAME    rowen_ipc
    TYPE    SHARED
    OUTPUT  TARGET
    SOURCES
        src/domain_listener.cpp
        src/domain_connector.cpp
    LINK_LIBS
        PUBLIC
            rt
            pthread
            ${PROJECT_NAME}_core
)
