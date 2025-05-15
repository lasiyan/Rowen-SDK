#pragma once

#ifndef rsdk_local_builder
  #include "rowen_system.h"
#endif

#include <cstdint>

namespace rs {

// Type definition
constexpr decltype(auto) ZERO_BYTES   = 0;
constexpr decltype(auto) UNSET_SIGNED = -1;
constexpr int8_t         INT8_FULL    = 0x7F;
constexpr int16_t        INT16_FULL   = 0x7FFF;
constexpr int32_t        INT32_FULL   = 0x7FFFFFFF;
constexpr int64_t        INT64_FULL   = 0x7FFFFFFFFFFFFFFF;
constexpr uint8_t        UINT8_FULL   = 0xFF;
constexpr uint16_t       UINT16_FULL  = 0xFFFF;
constexpr uint32_t       UINT32_FULL  = 0xFFFFFFFF;
constexpr uint64_t       UINT64_FULL  = 0xFFFFFFFFFFFFFFFF;

// CRC definition
constexpr uint32_t CRC8_INIT  = 0xFF;
constexpr uint32_t CRC16_INIT = 0xFFFF;
constexpr uint32_t CRC32_INIT = 0xFFFFFFFF;

};  // namespace rs
