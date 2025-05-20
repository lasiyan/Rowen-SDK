if (OPTION_WITH_JETSON_UTILS AND __platform_jetson__)
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
            ${PROJECT_NAME}_jetson
    )

    if (NOT OPTION_INSTALL_FOR_DEVELOPER)
        install(
            FILES ${CMAKE_CURRENT_SOURCE_DIR}/test.jpg
            DESTINATION ${RSDK_INSTALL_SAMPLEDIR}
        )
    endif()

    add_custom_command(TARGET ${TARGET} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy
        ${CMAKE_CURRENT_SOURCE_DIR}/test.jpg
        ${CMAKE_CURRENT_BINARY_DIR}/test.jpg
    )
endif()
