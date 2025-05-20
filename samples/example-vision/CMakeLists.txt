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

# Find the Jetson utils library
if (OPTION_WITH_JETSON_UTILS AND __platform_jetson__)
    target_link_libraries(${TARGET} PRIVATE ${PROJECT_NAME}_jetson)
endif()


# Install the example
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
