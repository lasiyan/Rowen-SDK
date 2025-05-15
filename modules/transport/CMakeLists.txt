rs_add_library(
    SOURCES
        src/packet_typedef.cpp
        src/packet_receiver.cpp
        src/socket_base.cpp
    OUTPUT TARGET
)

target_link_libraries(${TARGET}
    PUBLIC
        ${PROJECT_NAME}_core
)
