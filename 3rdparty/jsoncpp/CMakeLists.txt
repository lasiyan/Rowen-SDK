rs_add_library(
    # NAME rowen_jsoncpp
    SOURCES
        src/json_reader.cpp
        src/json_value.cpp
        src/json_writer.cpp
        src/json_helper.cpp
    OUTPUT TARGET
    INCLUDE_DIRS
        PUBLIC
            ${CMAKE_CURRENT_SOURCE_DIR}/include/${PROJECT_NAME}_3rd
)
