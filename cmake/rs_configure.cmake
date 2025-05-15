# Set default install prefix
if (CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
    set(CMAKE_INSTALL_PREFIX "${CMAKE_SOURCE_DIR}/install" CACHE PATH "SDK install prefix" FORCE)
endif()


# Set default build type
if (NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Build type" FORCE)
endif()


# Set global variables
include(GNUInstallDirs)
set(RSDK_INSTALL_INCLUDEDIR ${CMAKE_INSTALL_INCLUDEDIR})
set(RSDK_INSTALL_LIBDIR     ${CMAKE_INSTALL_LIBDIR})
set(RSDK_INSTALL_BINDIR     ${CMAKE_INSTALL_BINDIR})
set(RSDK_INSTALL_SHAREDIR   share/${PROJECT_NAME})
set(RSDK_INSTALL_SAMPLEDIR  sample)


# Overwrite default configures (for testing)
if (EXISTS ${CMAKE_CURRENT_LIST_DIR}/rs_configure.cmake.local)
    include(${CMAKE_CURRENT_LIST_DIR}/rs_configure.cmake.local)
endif()
