#ifndef ROWEN_SDK_CORE_DEFINE_HPP
#define ROWEN_SDK_CORE_DEFINE_HPP

#ifndef REX_SAFE_DELETE
  #define REX_SAFE_DELETE(ptr) \
    if (ptr)                   \
    {                          \
      delete ptr;              \
      ptr = nullptr;           \
    }
#endif

#ifndef REX_SAFE_DELETE_ARRAY
  #define REX_SAFE_DELETE_ARRAY(ptr) \
    if (ptr)                         \
    {                                \
      delete[] ptr;                  \
      ptr = nullptr;                 \
    }
#endif

#ifndef REX_HIWORD
  #define REX_HIWORD(l) ((uint16_t)((((uint32_t)(l)) >> 16) & 0xffff))
#endif

#ifndef REX_HIBYTE
  #define REX_HIBYTE(w) ((uint8_t)(((uint16_t)(w) >> 8) & 0xff))
#endif

#ifndef REX_LOWORD
  #define REX_LOWORD(l) ((uint16_t)(((uint32_t)(l)) & 0xffff))
#endif

#ifndef REX_LOBYTE
  #define REX_LOBYTE(w) ((uint8_t)(w))
#endif

#ifndef REX_SNPRINTF
  #define REX_SNPRINTF(buf, fmt, ...)               \
    {                                               \
      snprintf(buf, sizeof(buf), fmt, __VA_ARGS__); \
    }
#endif

#endif  // ROWEN_SDK_CORE_DEFINE_HPP
