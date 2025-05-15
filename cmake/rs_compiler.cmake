# Check compiler & Specialize
include(CheckCXXCompilerFlag)

set(CMAKE_CXX_STANDARD              17)
set(CMAKE_CXX_STANDARD_REQUIRED     ON)  # if compiler doesn't support C++11, stop
set(CMAKE_CXX_EXTENSIONS            OFF) # -std=c++ instead of -std=gnu++
set(CMAKE_EXPORT_COMPILE_COMMANDS   ON)  # compile_commands.json

# Test latest C++ Standard and High warning level to prevent mistakes

# --- MSVC
if(MSVC)
    message(STATUS "Traget Compiler : MSVC")
    check_cxx_compiler_flag(/W4             high_warning_level  )
    add_compile_options(/wd4710 /wd4996 /wd4477)

# --- Clang
elseif(${CMAKE_CXX_COMPILER_ID} MATCHES Clang)
    message(STATUS "Traget Compiler : Clang")
    check_cxx_compiler_flag(-Wall           high_warning_level  )
    add_compile_options(-Wno-unused-result)
    add_compile_options(-Wno-format-truncation)
    add_compile_options(-Wno-switch -Wno-comment)
    add_compile_options(-Wno-return-type-c-linkage)
    if (CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL 11)
        message(STATUS "Traget Compiler : " ${CMAKE_CXX_COMPILER_VERSION})
        add_compile_options(-Wno-deprecated-anon-enum-enum-conversion)
    endif()

# --- GNU
elseif(${CMAKE_CXX_COMPILER_ID} MATCHES GNU)
    message(STATUS "Traget Compiler : GNU")
    check_cxx_compiler_flag(-Wextra         high_warning_level  )
    add_compile_options(-Wno-unused-result)
    add_compile_options(-Wno-format-truncation)
    add_compile_options(-Wno-deprecated-declarations)
    add_compile_options(-Wno-return-type-c-linkage)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -rdynamic")
    if (CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL 11)
        message(STATUS "Traget Compiler : " ${CMAKE_CXX_COMPILER_VERSION})
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-deprecated-enum-enum-conversion")
    endif()
endif()
