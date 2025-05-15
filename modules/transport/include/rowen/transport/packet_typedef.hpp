#pragma once

#include <chrono>
#include <cstdint>
#include <cstring>
#include <string>
#include <vector>

#if defined(__GNUC__) || defined(__clang__)
  #define PACKED __attribute__((packed))
#elif defined(_MSC_VER)
  #define PACKED
  #pragma pack(push, 1)
#else  // Unknown Compiler
  #define PACKED
#endif

namespace rs {

class Packet
{
  static constexpr uint16_t UNDEFINED_VERSION   = 0x00;
  static constexpr uint8_t  UNDEFINED_TRANSPORT = 0X00;
  static constexpr uint64_t UNDEFINED_TIMESTAMP = 0x0000000000000000;
  static constexpr uint32_t UNDEFINED_OPCODE    = 0;
  static constexpr uint32_t UNDEFINED_SIZE      = 0;

 public:
  static constexpr uint8_t SOH = 0x01;
  static constexpr uint8_t STX = 0x02;
  static constexpr uint8_t ETX = 0x03;

 public:
  struct PACKED Header
  {
    const uint8_t SOH_ = SOH;

    struct PACKED Data
    {
      uint16_t version      = UNDEFINED_VERSION;    // LOBYTE: 메이저 버전, HIBYTE: 마이너 버전
      uint8_t  source       = UNDEFINED_TRANSPORT;  // 패킷을 보내는 쪽 (보내는 쪽에서 설정)
      uint8_t  destination  = UNDEFINED_TRANSPORT;  // 패킷을 받는 쪽
      uint64_t timestamp    = UNDEFINED_TIMESTAMP;  // 패킷을 보낸 시간 (마이크로초)
      uint32_t opcode       = UNDEFINED_OPCODE;     // 패킷의 종류 (오퍼레이션 코드)
      uint32_t total_size   = UNDEFINED_SIZE;       // 전체 패킷의 크기
      uint32_t payload_size = UNDEFINED_SIZE;       // 페이로드의 크기
      uint8_t  reserved[16] = {};                   // 예약 영역
    } data;

    const uint8_t STX_ = STX;
  };

  /*
  struct PACKED Payload
  {
    // Actual data
  }
  */

  struct PACKED Trailer
  {
    struct PACKED Data
    {
      uint32_t crc32        = 0x00000000;  // CRC32
      uint8_t  reserved[16] = {};
    } data;

    const uint8_t ETX_ = ETX;
  };

 public:
  static constexpr int     HEADER_SIZE  = sizeof(Header);
  static constexpr int     TRAILER_SIZE = sizeof(Trailer);
  static constexpr ssize_t PACKET_SIZE(const int payload_size) { return HEADER_SIZE + payload_size + TRAILER_SIZE; }
  static constexpr ssize_t PACKET_SIZE(const Header& packet_header) { return PACKET_SIZE(packet_header.data.payload_size); }

 public:
  Packet()                               = default;
  Packet(const Packet& other)            = delete;
  Packet& operator=(const Packet& other) = delete;
  virtual ~Packet();

  /**
   * @brief Make Packet with JSON
   */
  Packet(const int opcode, const std::string& json);

  /**
   * @brief Make Packet with Byte Stream
   */
  Packet(const uint8_t* packet_byte_stream, const size_t byte_stream_size);

  /**
   * @brief Make Packet with Byte Stream (from vector)
   */
  Packet(const std::vector<uint8_t>& packet_byte_stream);

  /**
   * @brief Make Packet
   */
  bool make(const uint16_t version, const uint8_t source, const uint8_t destination, const uint32_t opcode,
            const uint8_t* payload = nullptr, const size_t payload_size = 0);

  /**
   * @brief Make Packet from opcode and payload
   */
  bool make(const int opcode, const uint8_t* payload, const size_t payload_size);

  bool make(const int opcode, const std::string& json_data);

  /**
   * @brief Make Packet from other packet data
   */
  bool fill(const uint8_t* packet_byte_stream, const size_t byte_stream_size);

  /**
   * @brief Make Packet from other packet data (from vector)
   */
  bool fill(const std::vector<uint8_t>& packet_byte_stream);

  /**
   * @brief Check Packet validation
   */
  bool validation() const;

 private:
  static constexpr size_t OFFSET_VERSION     = sizeof(SOH);
  static constexpr size_t OFFSET_SOURCE      = OFFSET_VERSION + sizeof(Header::Data::version);
  static constexpr size_t OFFSET_DESTINATION = OFFSET_SOURCE + sizeof(Header::Data::source);
  static constexpr size_t OFFSET_TIMESTAMP   = OFFSET_DESTINATION + sizeof(Header::Data::destination);

 public:
  // Update
  void updateVersion(uint16_t version) const;
  void updateSource(uint8_t source) const;
  void updateDestination(uint8_t destination) const;
  void updateTimestamp(uint64_t microsec = current_time()) const;

  // Buffer
  const uint8_t* data() const { return buffer_; }
  const Header*  header() const { return buffer_ ? reinterpret_cast<const Header*>(buffer_) : nullptr; }
  const uint8_t* payload() const { return buffer_ ? buffer_ + HEADER_SIZE : nullptr; }
  const Trailer* trailer() const { return buffer_ ? reinterpret_cast<const Trailer*>(buffer_ + HEADER_SIZE + header()->data.payload_size) : nullptr; }

  // Size
  ssize_t size() const { return buffer_size_; }
  ssize_t header_size() const { return HEADER_SIZE; }
  ssize_t payload_size() const { return header() ? header()->data.payload_size : UNDEFINED_SIZE; }
  ssize_t trailer_size() const { return TRAILER_SIZE; }

 private:
  static uint64_t current_time() { return std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::system_clock::now().time_since_epoch()).count(); }

 private:
  uint8_t* buffer_      = nullptr;
  ssize_t  buffer_size_ = UNDEFINED_SIZE;
};

};  // namespace rs

#if defined(_MSC_VER)
  #pragma pack(pop)
#endif

#undef PACKED
