file(GLOB_RECURSE SRC_FILES CONFIGURE_DEPENDS src/*.cpp)

rs_add_library(
    # NAME    rowen_yamlcpp
    SOURCES ${SRC_FILES}
    OUTPUT  TARGET
    INCLUDE_DIRS
        PUBLIC
            ${CMAKE_CURRENT_SOURCE_DIR}/include/${PROJECT_NAME}_3rd
)
