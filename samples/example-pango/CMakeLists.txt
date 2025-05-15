rs_add_executable(
    TYPE SAMPLE
    SOURCES
        main.cpp
    OUTPUT TARGET
)

target_link_libraries(${TARGET}
    PRIVATE
        pthread
        opencv_core
        opencv_imgproc
        opencv_imgcodecs

        ${PROJECT_NAME}_core
        ${PROJECT_NAME}_vision
)

# Find pango
include(${CMAKE_SOURCE_DIR}/cmake/export/find_package_pango.cmake)
find_package_pango(TARGET ${TARGET})


# Find Jetson utils
if (OPTION_WITH_JETSON_UTILS AND __platform_jetson__)
    target_link_libraries(${TARGET} PRIVATE  ${PROJECT_NAME}_jetson)
endif()

# Install
install(
    FILES ${CMAKE_CURRENT_SOURCE_DIR}/test.jpg
    DESTINATION ${RSDK_INSTALL_SAMPLEDIR}
)

add_custom_command(TARGET ${TARGET} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy
    ${CMAKE_CURRENT_SOURCE_DIR}/test.jpg
    ${CMAKE_CURRENT_BINARY_DIR}/test.jpg
)
