# Check compiler & Specialize
include(CheckCXXCompilerFlag)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON) # if compiler doesn't support C++11, stop
set(CMAKE_CXX_EXTENSIONS OFF) # -std=c++ instead of -std=gnu++
set(CMAKE_EXPORT_COMPILE_COMMANDS 1) # compile_commands.json

# Test latest C++ Standard and High warning level to prevent mistakes
if(MSVC)
    message("-- Traget Compiler : MSVC")
    check_cxx_compiler_flag(/W4             high_warning_level  )
    add_compile_options(/wd4710 /wd4996 /wd4477)
elseif(${CMAKE_CXX_COMPILER_ID} MATCHES Clang)
    message("-- Traget Compiler : Clang")
    check_cxx_compiler_flag(-Wall           high_warning_level  )
    add_compile_options(-Wno-unused-result)
    add_compile_options(-Wno-format-truncation)
elseif(${CMAKE_CXX_COMPILER_ID} MATCHES GNU)
    message("-- Traget Compiler : GNU")
    check_cxx_compiler_flag(-Wextra         high_warning_level  )
    add_compile_options(-Wno-unused-result)
    add_compile_options(-Wno-format-truncation)
    add_compile_options(-Wno-deprecated-declarations)
    if (CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL 11)
        message("-- Traget Compiler : " ${CMAKE_CXX_COMPILER_VERSION})
        add_compile_options(-Wno-deprecated-enum-enum-conversion)
        message("-- set warning disabled : deprecated-enum-enum-conversion")
    endif()
endif()
