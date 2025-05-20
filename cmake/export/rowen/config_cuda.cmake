include(${CMAKE_CURRENT_LIST_DIR}/config_allocator.cmake)
# --------------------------------------------------------------------------------

function(cuda_version_configure OUTPUT_LIST)
    set(cuda_defines "${${OUTPUT_LIST}}") # backup previous defines

    # Set default cuda path
    set(cuda_base "${CMAKE_SYSROOT}/usr/local/")

    # find cuda-XX directory
    file(GLOB candidate_dirs "${cuda_base}cuda-*")
    message(DEBUG "CUDA directories found: ${candidate_dirs}")

    # (filter) cuda-<major>.<minor> format directories
    foreach(dir IN LISTS candidate_dirs)
        # extract version number
        string(REGEX MATCH "cuda-([0-9]+\\.[0-9]+)" matched "${dir}")
        if (matched)
            string(REGEX REPLACE "cuda-" "" cuda_version "${matched}")  # XX.Y
            message(STATUS "CUDA version     : ${cuda_version}") 

            string(REPLACE "." "_" cuda_version "${cuda_version}")      # XX_Y
            list(APPEND cuda_versions "${cuda_version}")
        endif()
    endforeach()

    # If List is empty, set default cuda version
    if (NOT cuda_versions)
        list(APPEND cuda_defines "__platform_cuda_disable__")
        list(APPEND cuda_defines "__unsupport_cuda__")
        message(STATUS "CUDA version     : Not found")
    else()
        list(APPEND cuda_defines "__platform_cuda_support__")
        foreach(dir IN LISTS cuda_versions)
            list(APPEND cuda_defines "__platform_cuda${dir}__")
        endforeach()
    endif()

    # Return
    set(${OUTPUT_LIST} "${cuda_defines}" PARENT_SCOPE)
endfunction()

cuda_version_configure(RS_CONFIG_FLAGS)
