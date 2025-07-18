cmake_minimum_required(VERSION 3.20)
project(rowen VERSION 2.0.4)


# SDK options definition
option(OPTION_BUILD_CORE_WITH_LOGGER    "Build Logger with Core Module"         OFF)
option(OPTION_WITH_DATABASE_LOGGER      "Enable Database Logger"                OFF)
option(OPTION_WITH_JETSON_UTILS         "Use Jetson Utils with Vision Module"   OFF)
option(OPTION_WITH_RSDK_VISION          "Enable Vision Module"                  OFF)
option(OPTION_EXPORT_CMAKE_SHARE        "Export rowen sdk configure cmake"      OFF)
option(OPTION_INSTALL_FOR_DEVELOPER     "Install include, lib, and share only"  OFF)
option(OPTION_INSTALL_TESTS             "Install test files"                    OFF)
set(OPTION_DEPLOY_PACKAGE_PATH          "" CACHE STRING "Export rowen sdk deploy package")


# Scripts
include(cmake/rs_configure.cmake)
include(cmake/rs_compiler.cmake)
include(cmake/rs_add_executable.cmake)
include(cmake/rs_add_library.cmake)
include(cmake/rs_post_install.cmake)
include(cmake/rs_config_generator.cmake)
include(cmake/export/RowenExport.cmake)

# Build configuration
add_compile_definitions(rsdk_local_builder)
rs_config_header_define_generator(RS_CONFIG_FLAGS)


# Sub directories
add_subdirectory(3rdparty)
add_subdirectory(modules)
add_subdirectory(samples)


# Export CMake configuration
if (OPTION_EXPORT_CMAKE_SHARE)
    install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/cmake/export/
        DESTINATION ${RSDK_INSTALL_SHAREDIR}
        FILES_MATCHING PATTERN "*.cmake"
    )
    # file(GLOB export_cmake_files cmake/export/*.cmake)
    # install(FILES ${export_cmake_files} DESTINATION ${RSDK_INSTALL_SHAREDIR})
endif()


# Deploy package
if (OPTION_DEPLOY_PACKAGE_PATH)
    install(CODE "
        execute_process(
            COMMAND ${CMAKE_COMMAND} 
                -DCMAKE_SYSROOT=${CMAKE_SYSROOT}
                -DBASE_DIRECTORY=${CMAKE_SOURCE_DIR}
                -DCMAKE_INSTALL_PREFIX=${CMAKE_INSTALL_PREFIX}
                -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
                -DDEPLOY_PACKAGE_PATH=${OPTION_DEPLOY_PACKAGE_PATH}
                -P ${CMAKE_SOURCE_DIR}/cmake/rs_deploy_script.cmake
        )
    "
    )
endif()
