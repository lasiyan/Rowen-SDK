rs_add_library(
    SOURCES
        src/json_helper.cpp
    OUTPUT TARGET
)

# Specify the library path
target_include_directories(${TARGET} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include/${PROJECT_NAME}_3rd
)
