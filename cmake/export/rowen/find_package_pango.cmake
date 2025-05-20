include(${CMAKE_CURRENT_LIST_DIR}/find_package.cmake)

# (optional) TARGET : target to include and link libraries
# (optional) SCOPE : scope of the package (PRIVATE, PUBLIC, INTERFACE)
# (optional) REQUIRED : package is required
# (optional) DEBUG : print debug information
# (result) Pango_FOUND : package is found
# (result) Pango_INCLUDE_DIRS : include directories
# (result) Pango_LIBS : libraries to link
macro(find_package_pango)
    set(LIB Pango)
    rs_find_package_initialize(${LIB} ${ARGN})

    # --- Include directories ---
    set(FIND_PATHS
        "${CMAKE_SYSROOT}/usr/include/pango-1.0"
        "${CMAKE_SYSROOT}/usr/include/fribidi"
        "${CMAKE_SYSROOT}/usr/include/cairo"
        "${CMAKE_SYSROOT}/usr/include/pixman-1"
        "${CMAKE_SYSROOT}/usr/include/uuid"
        "${CMAKE_SYSROOT}/usr/include/freetype2"
        "${CMAKE_SYSROOT}/usr/include/libpng16"
        "${CMAKE_SYSROOT}/usr/include/harfbuzz"
        "${CMAKE_SYSROOT}/usr/include/glib-2.0"
    )

    # Find Pango include directories
    foreach( path ${FIND_PATHS} )
        if(EXISTS ${path} AND IS_DIRECTORY ${path})
            list(APPEND ${LIB}_INCLUDE_DIRS ${path})
        endif()
    endforeach()

    # Check all find paths are valid
    list(LENGTH FIND_PATHS          i_target_count)
    list(LENGTH ${LIB}_INCLUDE_DIRS i_found_count)
    if (NOT i_target_count EQUAL i_found_count)
        message(STATUS "${LIB} include directories not found (${i_found_count}/${i_target_count})")
        set(${LIB}_INCLUDE_DIRS "")
        set(${LIB}_FOUND        FALSE)
        return()
    else()
        set(${LIB}_FOUND TRUE)    
    endif()

    # Check REQUIRED
    if (ARG_REQUIRED AND NOT ${LIB}_FOUND)
        message(FATAL_ERROR "${LIB} not found in ${FIND_PATHS}")
    endif()

    # --- Libraries ---
    if (${LIB}_FOUND)
        set(library_find_targets
            pangocairo-1.0
            pango-1.0
            gobject-2.0
            glib-2.0
            harfbuzz
            cairo
        )

        # Check if the library is found
        foreach( lib ${library_find_targets} )
            find_library(FOUND_${lib}
                NAMES ${lib}
                PATHS
                    "${CMAKE_SYSROOT}/usr/lib"
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
        target_compile_definitions(${ARG_TARGET}
            ${ARG_SCOPE}
                RS_VISION_WITH_PANGO
        )
    endif()
endmacro()
