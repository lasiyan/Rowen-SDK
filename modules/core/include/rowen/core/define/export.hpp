#pragma once

// --- Platform detection ---
#if defined _WIN32 || defined __CYGWIN__
  #define ROWEN_HELPER_DLL_IMPORT __declspec(dllimport)
  #define ROWEN_HELPER_DLL_EXPORT __declspec(dllexport)
  #define ROWEN_HELPER_DLL_LOCAL
#else
  #if __GNUC__ >= 4
    #define ROWEN_HELPER_DLL_IMPORT __attribute__((visibility("default")))
    #define ROWEN_HELPER_DLL_EXPORT __attribute__((visibility("default")))
    #define ROWEN_HELPER_DLL_LOCAL  __attribute__((visibility("hidden")))
  #else
    #define ROWEN_HELPER_DLL_IMPORT
    #define ROWEN_HELPER_DLL_EXPORT
    #define ROWEN_HELPER_DLL_LOCAL
  #endif
#endif

// --- Public API macro (import/export based on context) ---
#ifdef ROWEN_STATIC  // define if building/using static lib
  #define ROWEN_API
  #define ROWEN_LOCAL
#else
  #ifdef ROWEN_EXPORTS  // defined by build system when building DLL
    #define ROWEN_API ROWEN_HELPER_DLL_EXPORT
  #else
    #define ROWEN_API ROWEN_HELPER_DLL_IMPORT
  #endif
  #define ROWEN_LOCAL ROWEN_HELPER_DLL_LOCAL
#endif
