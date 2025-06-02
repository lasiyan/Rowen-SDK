rs_add_library(
    # NAME rowen_rapidjson
    SOURCES
        src/json_helper.cpp
    OUTPUT TARGET
    INCLUDE_DIRS
        PUBLIC
            ${CMAKE_CURRENT_SOURCE_DIR}/include/${PROJECT_NAME}_3rd
)
