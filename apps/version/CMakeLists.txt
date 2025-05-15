configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/version.h.in
    ${CMAKE_CURRENT_SOURCE_DIR}/version.h
)

set(TARGET ${PROJECT_NAME}_version)
rs_add_executable(
    NAME    ${TARGET}
    TYPE    APP
    SOURCES main.cpp
)
