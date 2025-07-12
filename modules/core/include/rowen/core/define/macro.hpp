#pragma once

// clang-format off

#ifndef RS_SAFE_DELETE
#define RS_SAFE_DELETE(ptr)           do { if (ptr) { delete ptr; ptr = nullptr; } } while (0)
#endif

#ifndef RS_SAFE_DELETE_ARRAY
#define RS_SAFE_DELETE_ARRAY(ptr)     do { if (ptr) { delete[] ptr; ptr = nullptr; } } while (0)
#endif

#ifndef RS_HIWORD
#define RS_HIWORD(l)                  ((uint16_t)((((uint32_t)(l)) >> 16) & 0xffff))
#endif

#ifndef RS_HIBYTE
#define RS_HIBYTE(w)                  ((uint8_t)(((uint16_t)(w) >> 8) & 0xff))
#endif

#ifndef RS_LOWORD
#define RS_LOWORD(l)                  ((uint16_t)(((uint32_t)(l)) & 0xffff))
#endif

#ifndef RS_LOBYTE
#define RS_LOBYTE(w)                  ((uint8_t)(w))
#endif

#ifndef RS_SNPRINTF
#define RS_SNPRINTF(buf, fmt, ...)    ::snprintf(buf, sizeof(buf), fmt, __VA_ARGS__)
#endif

// --- RETURN_IF_MACRO -----------------------------------------------------------
#ifndef RETURN_IF
#define _rs__return_if_impl(cond, ...)            do { if ((cond)) { return __VA_ARGS__; } } while (0)
#define RETURN_IF(...)                            _rs__return_if_impl(__VA_ARGS__)
#endif
#if !defined(return_if) && defined(RETURN_IF)     // Alias for RETURN_IF
#define return_if(...)                            RETURN_IF(__VA_ARGS__)
#endif

// clang-format on
