macro(rs_find_package_initialize NAME)

    set(options
            NO_DEFAULT_PATH
            REQUIRED
            DEBUG
    )

    set(oneValueArgs
            TARGET
            SCOPE # PRIVATE, PUBLIC, INTERFACE
    )

    set(multiValueArgs 
            PATHS
    )
    cmake_parse_arguments(ARG "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

    # --- Set default values ---
    # SCOPE
    if (NOT ARG_SCOPE)
        set(ARG_SCOPE PRIVATE)
    endif()

    # --- Initialize result variables ---
    set(${NAME}_FOUND           FALSE)
    set(${NAME}_INCLUDE_DIRS    "")
    set(${NAME}_LIBS            "")

endmacro()
