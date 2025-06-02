rs_add_library(
    # NAME    rowen_rapidxml
    TYPE    INTERFACE
    OUTPUT  TARGET
    INCLUDE_DIRS
        INTERFACE
            ${CMAKE_CURRENT_SOURCE_DIR}/include/${PROJECT_NAME}_3rd
)
