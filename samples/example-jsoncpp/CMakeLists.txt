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
        ${PROJECT_NAME}_jsoncpp
)

if (NOT OPTION_INSTALL_FOR_DEVELOPER)
    install(
        FILES ${CMAKE_CURRENT_SOURCE_DIR}/test.json
        DESTINATION ${RSDK_INSTALL_SAMPLEDIR}
    )
endif()

add_custom_command(TARGET ${TARGET} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy
    ${CMAKE_CURRENT_SOURCE_DIR}/test.json
    ${CMAKE_CURRENT_BINARY_DIR}/test.json
)
