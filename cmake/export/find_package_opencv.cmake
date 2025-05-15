include(${CMAKE_CURRENT_LIST_DIR}/find_package.cmake)

# (optional) TARGET : target to include and link libraries
# (optional) PATHS : paths to search for the package
# (optional) SCOPE : scope of the package (PRIVATE, PUBLIC, INTERFACE)
# (optional) NO_DEFAULT_PATH : do not search in the default paths
# (optional) REQUIRED : package is required
# (optional) DEBUG : print debug information
# (result) OpenCV_INCLUDE_DIRS : include directories
# (result) OpenCV_LIBS : libraries to link
# (result) OpenCV_FOUND : package is found
macro(find_package_opencv)
    set(LIB OpenCV)
    rs_find_package_initialize(${LIB} ${ARGN})

    # --- Include directories ---
    set(FIND_PATHS
        "${CMAKE_SYSROOT}/usr/include/opencv4"
        "${CMAKE_SYSROOT}/usr/local/include/opencv4"    
    )

    # If NO_DEFAULT_PATH is set, clear the default paths
    if (ARG_NO_DEFAULT_PATH)
        set(FIND_PATHS "")
    endif()

    # Check PATHS
    if(ARG_PATHS)
        foreach(path ${ARG_PATHS})
            # Check if the path is not already in the list
            if (NOT path IN_LIST FIND_PATHS)
                list(APPEND FIND_PATHS ${path})
            endif()
        endforeach()
    endif()

    # Find OpenCV include directories
    foreach( path ${FIND_PATHS} )
        if(EXISTS ${path} AND IS_DIRECTORY ${path})
            if (EXISTS ${path}/opencv2/core.hpp) # Check if the library core file exists
                set(${LIB}_FOUND        TRUE)
                set(${LIB}_INCLUDE_DIRS ${path})
                break()
            endif()            
        endif()
    endforeach()
    if (ARG_DEBUG)
        message(STATUS "${LIB}_INCLUDE_DIRS : ${${LIB}_INCLUDE_DIRS}")
    endif()

    # Check REQUIRED
    if (ARG_REQUIRED AND NOT ${LIB}_FOUND)
        message(FATAL_ERROR "${LIB} not found in ${FIND_PATHS}")
    endif()

    # --- Libraries ---
    if (${LIB}_FOUND)
        set(library_find_targets
            opencv_core
            opencv_imgproc
            opencv_highgui
            opencv_imgcodecs
            opencv_videoio
            opencv_video
        )

        # Check if the library is found
        foreach( lib ${library_find_targets} )
            find_library(FOUND_${lib}
                NAMES ${lib}
                PATHS
                    "${CMAKE_SYSROOT}/usr/lib/${CMAKE_LIBRARY_ARCHITECTURE}"
            )

            if (FOUND_${lib})
                list(APPEND ${LIB}_LIBS ${FOUND_${lib}})
            else()
                message(WARNING "${LIB} library not found: ${lib}")
            endif()
        endforeach()

        # Check all libraries found
        list(LENGTH library_find_targets target_count)
        list(LENGTH ${LIB}_LIBS          found_count)
        if (NOT target_count EQUAL found_count)
            message(WARNING "${LIB} library not found (${found_count}/${target_count})")
            set(${LIB}_FOUND FALSE)
            set(${LIB}_LIBS  "")
            return()
        endif()
    endif()

    # --- Set result variables ---
    if (${LIB}_FOUND AND ARG_TARGET)
        target_include_directories(${ARG_TARGET}
            ${ARG_SCOPE}
                ${${LIB}_INCLUDE_DIRS}
        )
        target_link_libraries(${ARG_TARGET}
            ${ARG_SCOPE}
                ${${LIB}_LIBS}
        )
    endif()
endmacro()
