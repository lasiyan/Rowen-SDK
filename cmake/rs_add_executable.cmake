function(rs_add_executable)
    # Option parser
    include(CMakeParseArguments)
    set(options)
    set(oneValueArgs
            NAME # Executable name (optional)
            OUTPUT # Created Library name
            OUTPUT_NAME # Alias for OUTPUT
            TYPE # Executable type. SAMPLE, APP etc. (optional) 
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
        set(ARG_NAME ${folder_name})  # Set the name to the last part of the path (with PROJECT_NAME)
    endif()

    # If TYPE is not specified, use default as SHARED
    if (ARG_TYPE STREQUAL "Sample" OR ARG_TYPE STREQUAL "sample" OR ARG_TYPE STREQUAL "Example" OR ARG_TYPE STREQUAL "example")
        set(ARG_TYPE SAMPLE)
    elseif(ARG_TYPE STREQUAL "App" OR ARG_TYPE STREQUAL "app" OR ARG_TYPE STREQUAL "Application" OR ARG_TYPE STREQUAL "application")
        set(ARG_TYPE APP)
    elseif(NOT ARG_TYPE)
        message(STATUS "Executable type not specified. Defaulting to SAMPLE.")
        return()
    endif()

    # Add executable
    add_executable(${ARG_NAME} ${ARG_SOURCES})
    message(STATUS "Include execute : ${ARG_NAME} (${ARG_TYPE})")

    # Set executable properties
    set_target_properties(${ARG_NAME} PROPERTIES
        INSTALL_RPATH "$ORIGIN/../${RSDK_INSTALL_LIBDIR}"
    )

    # Set output name
    set(${ARG_OUTPUT_NAME}  ${ARG_NAME} PARENT_SCOPE)
    set(${ARG_OUTPUT}       ${ARG_NAME} PARENT_SCOPE)

    # Set Insall directory (APP)
    if (NOT OPTION_INSTALL_FOR_DEVELOPER)
        if (ARG_TYPE STREQUAL "APP")
            install(
                TARGETS     ${ARG_NAME}
                DESTINATION ${RSDK_INSTALL_BINDIR}
                PERMISSIONS ${INSTALL_EXECUTE_PERMISSION}
            )    
        elseif (ARG_TYPE STREQUAL "SAMPLE")
            install(
                TARGETS     ${ARG_NAME}
                DESTINATION ${RSDK_INSTALL_SAMPLEDIR}
                PERMISSIONS ${INSTALL_EXECUTE_PERMISSION}
            )
        endif()
    endif()
endfunction()
