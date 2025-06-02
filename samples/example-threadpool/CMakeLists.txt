rs_add_executable(
    TYPE SAMPLE
    SOURCES
        main.cpp
    OUTPUT TARGET
)

target_link_libraries(${TARGET} PRIVATE pthread ${PROJECT_NAME}_core ${PROJECT_NAME}_utils)
if (NOT OPTION_BUILD_CORE_WITH_LOGGER)
    target_link_libraries(${TARGET} PRIVATE ${PROJECT_NAME}_logger)
endif()
