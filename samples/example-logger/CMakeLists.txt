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
        ${PROJECT_NAME}_logger
)
