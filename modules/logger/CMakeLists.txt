rs_add_library(
    SOURCES
        src/logger_api.cpp
        src/logger_base.cpp
        src/logger_console.cpp
        src/logger_file.cpp
        src/logger_websvc.cpp
    OUTPUT TARGET
)

target_link_libraries(${TARGET}
    PUBLIC
        ${PROJECT_NAME}_core
)

if (OPTION_WITH_DATABASE_LOGGER)
    set(RS_WITH_DATABASE_LOGGER True)
    target_link_libraries(${TARGET}
        PUBLIC
            ${PROJECT_NAME}_jsoncpp
            ${PROJECT_NAME}_mariadb
    )
endif()

# Make configure
configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/include/${PROJECT_NAME}/logger/template/logger_config.h.in
    ${CMAKE_CURRENT_SOURCE_DIR}/include/${PROJECT_NAME}/logger/template/logger_config.h
)
