rs_add_executable(
    TYPE SAMPLE
    SOURCES
        main.cpp
    OUTPUT TARGET
)

target_link_libraries(${TARGET}
    PRIVATE
        pthread
        ${PROJECT_NAME}_core
        ${PROJECT_NAME}_vision
)

# Set geometry test
set(TESTING_GEOMETRY_OPENCV ON)

if (TESTING_GEOMETRY_OPENCV)
    target_link_libraries(${TARGET} 
        PRIVATE
            opencv_core
            opencv_imgproc
            opencv_imgcodecs
    )

    target_compile_definitions(${TARGET}
        PRIVATE
            TESTING_GEOMETRY_OPENCV
    )
  
    if (NOT OPTION_INSTALL_FOR_DEVELOPER)
        install(
            FILES ${CMAKE_CURRENT_SOURCE_DIR}/test.jpg
            DESTINATION ${RSDK_INSTALL_SAMPLEDIR})
    endif()

    add_custom_command(TARGET ${TARGET} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy
            ${CMAKE_CURRENT_SOURCE_DIR}/test.jpg
            ${CMAKE_CURRENT_BINARY_DIR}/test.jpg
    )
endif()
