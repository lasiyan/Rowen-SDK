function(rs_add_library)
    # Option parser
    include(GNUInstallDirs)
    include(CMakeParseArguments)
    set(options)
    set(oneValueArgs
            NAME # Library name (optional)
            TYPE # SHARED or STATIC or INTERFACE
            OUTPUT # Created Library name
            OUTPUT_NAME # Alias for OUTPUT
    )
    set(multiValueArgs
            SOURCES # Source files
    )
    cmake_parse_arguments(ARG "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

    # If NAME is not specified, use the current directory name
    if(NOT ARG_NAME)
        get_filename_component(current_path "${CMAKE_CURRENT_LIST_FILE}" DIRECTORY) # Get CMakeLists.txt path
        get_filename_component(current_path "${current_path}" NAME) # Get the last part of the path
        string(REPLACE " " "_" folder_name  "${current_path}")  # Replace space with underscore
        string(REPLACE "-" "_" folder_name  "${folder_name}")  # Replace dash with underscore
        set(ARG_NAME ${PROJECT_NAME}_${folder_name})  # Set the name to the last part of the path (with PROJECT_NAME)
    endif()

    # If TYPE is not specified, use default as SHARED
    if(NOT ARG_TYPE)
        set(ARG_TYPE SHARED)
    endif()

    # Add library
    add_library(${ARG_NAME} ${ARG_TYPE} ${ARG_SOURCES})
    message(STATUS "Include library : ${ARG_NAME} (${ARG_TYPE})")

    # Set library properties
    if (ARG_TYPE STREQUAL "SHARED" OR ARG_TYPE STREQUAL "STATIC")
        set_target_properties(${ARG_NAME} PROPERTIES
            POSITION_INDEPENDENT_CODE ON
            INSTALL_RPATH "$ORIGIN"
        )
        target_include_directories(${ARG_NAME} PUBLIC
            $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/include>
            $<INSTALL_INTERFACE:${RSDK_INSTALL_INCLUDEDIR}>
        )
    elseif (ARG_TYPE STREQUAL "INTERFACE")
        target_include_directories(${ARG_NAME} INTERFACE
            $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/include>
            $<INSTALL_INTERFACE:${RSDK_INSTALL_INCLUDEDIR}>
        )
    else()
        message(FATAL_ERROR "Invalid library type: ${ARG_TYPE}. Use SHARED, STATIC, or INTERFACE.")
    endif()

    # Set output name
    set(${ARG_OUTPUT_NAME}  ${ARG_NAME} PARENT_SCOPE)
    set(${ARG_OUTPUT}       ${ARG_NAME} PARENT_SCOPE)

    # Set Insall directory (LIBRARY)
    install(
        DIRECTORY   ${CMAKE_CURRENT_LIST_DIR}/include/
        DESTINATION ${RSDK_INSTALL_INCLUDEDIR}
        FILES_MATCHING
            PATTERN "*.h"
            PATTERN "*.hpp"
            PATTERN "*.inl"
            PATTERN ".temp*"    EXCLUDE
            PATTERN ".tmp*"     EXCLUDE
            PATTERN ".test*"    EXCLUDE
            PATTERN ".back* "   EXCLUDE
            PATTERN ".bak*"     EXCLUDE
    )
    install(
        TARGETS ${ARG_NAME}
        EXPORT ${ARG_OUTPUT}Config
        RUNTIME DESTINATION ${RSDK_INSTALL_BINDIR}
        LIBRARY DESTINATION ${RSDK_INSTALL_LIBDIR}
        ARCHIVE DESTINATION ${RSDK_INSTALL_LIBDIR}
    )

endfunction()
