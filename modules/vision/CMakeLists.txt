rs_add_library(
    TYPE    SHARED
    OUTPUT  TARGET
    SOURCES
        # src/frame_casting.cpp
        src/polygon_impl.cpp
        src/drawable_pango.cpp
        src/converter_bgr.cpp
    OUTPUT TARGET
    LINK_LIBS
        PUBLIC
            pthread
            ${PROJECT_NAME}_core
)


# OpenCV
find_package_opencv(TARGET ${TARGET} SCOPE PUBLIC REQUIRED)


# Pango Library
find_package_pango(TARGET ${TARGET})


# Jetson utils
if (OPTION_WITH_JETSON_UTILS AND __platform_jetson__)
    target_compile_definitions(${TARGET} PRIVATE RSDK_WITH_JETSON_UTILS)
endif()
