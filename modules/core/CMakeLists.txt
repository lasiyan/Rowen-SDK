# Build with logger
if (OPTION_BUILD_CORE_WITH_LOGGER)
    set(RS_LOGGER_OBJCETS $<TARGET_OBJECTS:logger_obj>)
    set(RS_LOGGER_INCLUDE $<TARGET_PROPERTY:logger_obj,INTERFACE_INCLUDE_DIRECTORIES>)
endif()


# Generate version.cpp
configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/src/version.cpp.in
    ${CMAKE_CURRENT_SOURCE_DIR}/src/version.cpp
)


rs_add_library(
    # NAME    rowen_core
    TYPE    SHARED
    OUTPUT  TARGET
    SOURCES
        src/string.cpp
        src/system.cpp
        src/pretty_function.cpp
        src/time.cpp
        src/characterSet.cpp
        src/version.cpp
        ${RS_LOGGER_OBJCETS}

        src/transport/packet_typedef.cpp
        src/transport/packet_receiver.cpp
    INCLUDE_DIRS
        PUBLIC
            ${RS_LOGGER_INCLUDE}
)
