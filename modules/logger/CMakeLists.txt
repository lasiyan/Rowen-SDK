# Glob Logger sources
set(LOGGER_SOURCES
    src/logger_api.cpp
    src/logger_base.cpp
    src/logger_console.cpp
    src/logger_file.cpp
    src/logger_websvc.cpp
)

# Configure logger definitions
configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/include/${PROJECT_NAME}/logger/detail/logger_config.hpp.in
    ${CMAKE_CURRENT_SOURCE_DIR}/include/${PROJECT_NAME}/logger/detail/logger_config.hpp
)

# Make objects
rs_add_library(
    NAME    logger_obj
    TYPE    OBJECT
    OUTPUT  TARGET
    SOURCES ${LOGGER_SOURCES}
    INCLUDE_DIRS
        PUBLIC
            $<TARGET_PROPERTY:${PROJECT_NAME}_core,INTERFACE_INCLUDE_DIRECTORIES>
)

# (optional) Database logger
if (OPTION_WITH_DATABASE_LOGGER)
    target_compile_definitions(logger_obj PUBLIC RS_WITH_DATABASE_LOGGER)
    target_link_libraries(logger_obj PUBLIC ${PROJECT_NAME}_jsoncpp ${PROJECT_NAME}_mariadb)
endif()


# --- Build with `rowen_core` or not ---------------------------------------------
if (NOT OPTION_BUILD_CORE_WITH_LOGGER)
    rs_add_library(
        # NAME    rowen_logger
        TYPE    SHARED
        OUTPUT  TARGET
        SOURCES
            $<TARGET_OBJECTS:logger_obj>
        LINK_LIBS
            PUBLIC
                ${PROJECT_NAME}_core
    )
endif()
