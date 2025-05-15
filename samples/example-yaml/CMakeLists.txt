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
        ${PROJECT_NAME}_yamlcpp
)

install(
    FILES ${CMAKE_CURRENT_SOURCE_DIR}/test.yaml
    DESTINATION ${RSDK_INSTALL_SAMPLEDIR}
)

add_custom_command(TARGET ${TARGET} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy
    ${CMAKE_CURRENT_SOURCE_DIR}/test.yaml
    ${CMAKE_CURRENT_BINARY_DIR}/test.yaml
)
