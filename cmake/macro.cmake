# argv0 : file & path
macro(setup)
    get_filename_component(module ${CMAKE_CURRENT_LIST_DIR} NAME)

    # header
    if (${ARGV0} MATCHES "\\.h(pp)?|\\.inl$")
        if (DEFINED ROWEN_SDK_INSTALL)
            # set target directory : .../{__include_path}/{module}/...
            string(REPLACE "${CMAKE_CURRENT_LIST_DIR}/" "" target_temp ${ARGV0}) # file/opencv
            set(target "${__include_path}/${module}/${target_temp}") # .../{__include_path}/{module}/file/opencv/files.hpp

            # make sure target directory exists
            get_filename_component(target_dir ${target} DIRECTORY) # .../{__include_path}/{module}/file/opencv
            file(MAKE_DIRECTORY ${target_dir})

            # copy file from ARGV0 to target
            configure_file(${ARGV0} ${target} COPYONLY)
        else()
            message("Skip : debug mode : ${ARGV0}")
        endif()

    # source
    elseif(${ARGV0} MATCHES "\\.c(pp)?$")
        list(APPEND sdk_sources ${ARGV0})
        set(sdk_sources ${sdk_sources} PARENT_SCOPE)

    # not defined    
    else()
        message("Invalid file extension : ${ARGV0}")
    endif()
endmacro()

macro(setup_root)
    if (${ARGV0} MATCHES "\\.h(pp)?|\\.inl$")
        if (DEFINED ROWEN_SDK_INSTALL)
            get_filename_component(target_dir ${ARGV0} DIRECTORY)
            file(MAKE_DIRECTORY ${target_dir})

            configure_file(${ARGV0} ${__include_path} COPYONLY)
        else()
            message("Skip : debug mode : ${ARGV0}")
            return()
        endif()
    endif()
endmacro()

macro(copyFile)
  set(CopyFrom ${ARGV0})

  get_filename_component(CopyToDir ${ARGV1} DIRECTORY)
  if(NOT EXISTS ${CopyToDir})
    file(MAKE_DIRECTORY ${CopyToDir})
  endif()
  
  set(CopyTo   ${ARGV1})
  configure_file(${CopyFrom} ${CopyTo} COPYONLY)
endmacro(copyFile)

macro(get_directory_name PROJECT_NAME_VAR)
    get_filename_component(CURRENT_LIST_FILE "${CMAKE_CURRENT_LIST_FILE}" DIRECTORY)
    get_filename_component(FOLDER_TITLE "${CURRENT_LIST_FILE}" NAME)
    set(${PROJECT_NAME_VAR} ${FOLDER_TITLE})
endmacro()

macro(get_target_name PROJECT_NAME_VAR)
    get_directory_name(DIRECTORY_NAME)
    set(${PROJECT_NAME_VAR} ${PREFIX}_${DIRECTORY_NAME})
endmacro()
