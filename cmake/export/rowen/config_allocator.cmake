function(rs_config_cmake_variable_allocate CONFIG_FLAGS)
    foreach(elem IN LISTS ${CONFIG_FLAGS})
        string(FIND ${elem} "=" epos)

        if(epos GREATER -1)
            string(SUBSTRING ${elem} 0 ${epos} macro_name)
            math(EXPR vpos "${epos} + 1")
            string(SUBSTRING ${elem} ${vpos} -1 macro_value)

            # Set CMake configure flags
            message(STATUS "Global flags : ${macro_name} => ${macro_value}")
            set(${macro_name} ${macro_value} CACHE STRING "CMake global variable" FORCE)
        else()
            # Set CMake configure flags
            message(STATUS "Global flags : ${elem} => TRUE")
            set(${elem} TRUE CACHE BOOL "${elem} flag" FORCE)
        endif()
    endforeach()

endfunction()
