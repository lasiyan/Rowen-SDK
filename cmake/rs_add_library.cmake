function(rs_add_library)

    # Set CMake Options
    include(CMakeParseArguments)
    set(options
            TEST_SUPRESS # Suppress test output
            TEST_INSTALL # Install test files
    )
    set(oneValueArgs
            NAME # Library name (default: last part of the path)
            TYPE # SHARED or STATIC or INTERFACE or OBJECT (default: SHARED)
            OUTPUT # Created Library name
            OUTPUT_NAME # Alias for OUTPUT
    )
    set(multiValueArgs
            SOURCES # Source files
            INCLUDE_DIRS
            LINK_DIRS
            LINK_LIBS
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
    if (ARG_TYPE STREQUAL "SHARED" OR ARG_TYPE STREQUAL "STATIC" OR ARG_TYPE STREQUAL "OBJECT")
        set_target_properties(${ARG_NAME} PROPERTIES
            POSITION_INDEPENDENT_CODE   ON
            INSTALL_RPATH               "$ORIGIN/../lib"
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


    # Set Library include and linking
    __rs_include_with_add_library(${ARG_NAME})
    __rs_linkdir_with_add_library(${ARG_NAME})
    __rs_linking_with_add_library(${ARG_NAME})


    # --- Library tests ---------------------------------------------------------------------------------------------------
    if (OPTION_INSTALL_TESTS OR ARG_TEST_INSTALL)
        if (NOT ARG_TEST_SUPRESS)
            # If test directory exists, add tests
            set(test_dir "${CMAKE_CURRENT_LIST_DIR}/test")
            if (EXISTS ${test_dir})
                # get all test files
                file(GLOB_RECURSE test_files "${test_dir}/*.cpp")

                if (test_files)
                    set(test_target "${ARG_NAME}_test")
                    add_executable(${test_target} ${test_files})

                    target_link_libraries(${test_target}
                        PRIVATE
                            ${ARG_NAME}
                            gtest_main
                    )
                    __rs_include_with_add_library(${test_target})
                    __rs_linkdir_with_add_library(${test_target})
                    __rs_linking_with_add_library(${test_target})

                    set_target_properties(${test_target} PROPERTIES
                        INSTALL_RPATH "$ORIGIN/../lib"
                    )

                    add_test(NAME ${test_target} COMMAND ${test_target})
                    message(STATUS "Include library : ${test_target}")
                    set(${ARG_OUTPUT_TEST} ${test_target} PARENT_SCOPE)

                    # Install test files
                    if (ARG_TEST_INSTALL)
                        install(
                            TARGETS ${test_target}
                            RUNTIME DESTINATION ${RSDK_INSTALL_BINDIR}
                            LIBRARY DESTINATION ${RSDK_INSTALL_LIBDIR}
                            ARCHIVE DESTINATION ${RSDK_INSTALL_LIBDIR}
                        )
                    endif()
                endif()
            endif()
        endif()
    endif()
    # --- End of Library tests --------------------------------------------------------------------------------------------


    # Return library name
    set(${ARG_OUTPUT} ${ARG_NAME} PARENT_SCOPE)


    # setup install
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

    if (ARG_TYPE STREQUAL "SHARED")
        install(
            TARGETS ${ARG_NAME}
            RUNTIME DESTINATION ${RSDK_INSTALL_BINDIR}
            LIBRARY DESTINATION ${RSDK_INSTALL_LIBDIR}
            ARCHIVE DESTINATION ${RSDK_INSTALL_LIBDIR}
        )
    endif()
endfunction()

# --------------------------------------------------------------------------------
# Helper functions for include, link directories, and linking
# --------------------------------------------------------------------------------
macro(__rs_linking_with_add_library LINK_TARGET)
    if(ARG_LINK_LIBS)
        set(curr_visibility "")
        set(libs_PRIVATE    "")
        set(libs_PUBLIC     "")
        set(libs_INTERFACE  "")

        foreach(item IN LISTS ARG_LINK_LIBS)
            if(item STREQUAL "PUBLIC" OR item STREQUAL "PRIVATE" OR item STREQUAL "INTERFACE")
                if (${ARG_TYPE} STREQUAL "INTERFACE")
                    set(curr_visibility "INTERFACE")
                else()
                    if (item STREQUAL "INTERFACE")
                        set(curr_visibility "PRIVATE")
                    else()
                        set(curr_visibility ${item})
                    endif()
                endif()
            elseif(curr_visibility STREQUAL "")
                if(${ARG_TYPE} STREQUAL "INTERFACE")
                    set(curr_visibility "INTERFACE")
                else()
                    set(curr_visibility "PRIVATE")
                endif()
                list(APPEND "libs_${curr_visibility}" ${item})
            else()
                list(APPEND "libs_${curr_visibility}" ${item})
            endif()
        endforeach()

        if (libs_PRIVATE)
            target_link_libraries(${LINK_TARGET} PRIVATE ${libs_PRIVATE})
        endif()
        if (libs_PUBLIC)
            target_link_libraries(${LINK_TARGET} PUBLIC ${libs_PUBLIC})
        endif()
        if (libs_INTERFACE)
            target_link_libraries(${LINK_TARGET} INTERFACE ${libs_INTERFACE})
        endif()
    endif() # ARG_INCLUDE_DIRS
endmacro()

macro(__rs_include_with_add_library INCLUDE_TARGET)
    if(ARG_INCLUDE_DIRS)
        set(curr_visibility "")
        set(dirs_PRIVATE    "")
        set(dirs_PUBLIC     "")
        set(dirs_INTERFACE  "")

        foreach(item IN LISTS ARG_INCLUDE_DIRS)
            if(item STREQUAL "PUBLIC" OR item STREQUAL "PRIVATE" OR item STREQUAL "INTERFACE")
                if (${ARG_TYPE} STREQUAL "INTERFACE")
                    set(curr_visibility "INTERFACE")
                else()
                    if (item STREQUAL "INTERFACE")
                        set(curr_visibility "PRIVATE")
                    else()
                        set(curr_visibility ${item})
                    endif()
                endif()
            elseif(curr_visibility STREQUAL "")
                if(${ARG_TYPE} STREQUAL "INTERFACE")
                    set(curr_visibility "INTERFACE")
                else()
                    set(curr_visibility "PRIVATE")
                endif()
                list(APPEND "dirs_${curr_visibility}" ${item})
            else()
                list(APPEND "dirs_${curr_visibility}" ${item})
            endif()
        endforeach()

        if (dirs_PRIVATE)
            target_include_directories(${INCLUDE_TARGET} PRIVATE ${dirs_PRIVATE})
        endif()
        if (dirs_PUBLIC)
            target_include_directories(${INCLUDE_TARGET} PUBLIC ${dirs_PUBLIC})
        endif()
        if (dirs_INTERFACE)
            target_include_directories(${INCLUDE_TARGET} INTERFACE ${dirs_INTERFACE})
        endif()
    endif() # ARG_INCLUDE_DIRS
endmacro()

macro(__rs_linkdir_with_add_library LINKDIR_TARGET)
    if(ARG_LINK_DIRS)
        set(curr_visibility "")
        set(dirs_PRIVATE    "")
        set(dirs_PUBLIC     "")
        set(dirs_INTERFACE  "")

        foreach(item IN LISTS ARG_LINK_DIRS)
            if(item STREQUAL "PUBLIC" OR item STREQUAL "PRIVATE" OR item STREQUAL "INTERFACE")
                if (${ARG_TYPE} STREQUAL "INTERFACE")
                    set(curr_visibility "INTERFACE")
                else()
                    if (item STREQUAL "INTERFACE")
                        set(curr_visibility "PRIVATE")
                    else()
                        set(curr_visibility ${item})
                    endif()
                endif()
            elseif(curr_visibility STREQUAL "")
                if(${ARG_TYPE} STREQUAL "INTERFACE")
                    set(curr_visibility "INTERFACE")
                else()
                    set(curr_visibility "PRIVATE")
                endif()
                list(APPEND "dirs_${curr_visibility}" ${item})
            else()
                list(APPEND "dirs_${curr_visibility}" ${item})
            endif()
        endforeach()

        if (dirs_PRIVATE)
            target_link_directories(${LINKDIR_TARGET} PRIVATE ${dirs_PRIVATE})
        endif()
        if (dirs_PUBLIC)
            target_link_directories(${LINKDIR_TARGET} PUBLIC ${dirs_PUBLIC})
        endif()
        if (dirs_INTERFACE)
            target_link_directories(${LINKDIR_TARGET} INTERFACE ${dirs_INTERFACE})
        endif()
    endif() # ARG_LINK_DIRS
endmacro()
