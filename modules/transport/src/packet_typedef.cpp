
#include "rowen/transport/packet_typedef.hpp"

#include "rowen/core/define/macro.hpp"

// #include "../define/typedef.hpp"

namespace rs {

Packet::~Packet()
{
  RS_SAFE_DELETE_ARRAY(buffer_);
}

Packet::Packet(const int opcode, const std::string& json)
{
  make(opcode, json);
}

Packet::Packet(const uint8_t* packet_byte_stream, const size_t byte_stream_size)
{
  fill(packet_byte_stream, byte_stream_size);
}

Packet::Packet(const std::vector<uint8_t>& packet_byte_stream) : Packet(packet_byte_stream.data(), packet_byte_stream.size())
{
}

bool Packet::make(const uint16_t version, const uint8_t source, const uint8_t destination, const uint32_t opcode,
                  const uint8_t* payload, const size_t payload_size)
{
  if (payload == nullptr || payload_size <= UNDEFINED_SIZE)
    return false;

  // Reset buffer
  RS_SAFE_DELETE_ARRAY(buffer_);

  // Get total packet size & allocate buffer
  auto packet_size = PACKET_SIZE(payload_size);
  buffer_          = new uint8_t[packet_size];
  buffer_size_     = packet_size;

  // Fill Header
  // clang-format off
    struct Header header;
    if (version > UNDEFINED_VERSION)        header.data.version = version;
    if (source > UNDEFINED_TRANSPORT)       header.data.source = source;
    if (destination > UNDEFINED_TRANSPORT)  header.data.destination = destination;
    if (opcode > UNDEFINED_OPCODE)          header.data.opcode = opcode;
    header.data.total_size = packet_size;
    header.data.payload_size = payload_size;
    header.data.timestamp = current_time();
  // clang-format on
  std::memcpy(buffer_, &header, HEADER_SIZE);

  // Fill Payload
  std::memcpy(buffer_ + HEADER_SIZE, payload, payload_size);

  // Fill Trailer
  struct Trailer trailer;
  trailer.data.crc32 = 0;  // TODO : Calculate CRC
  std::memcpy(buffer_ + HEADER_SIZE + payload_size, &trailer, TRAILER_SIZE);

  return true;
}

bool Packet::make(const int opcode, const uint8_t* payload, const size_t payload_size)
{
  return make(UNDEFINED_VERSION,
              UNDEFINED_TRANSPORT,
              UNDEFINED_TRANSPORT,
              static_cast<uint32_t>(opcode),
              payload,
              static_cast<uint32_t>(payload_size));
}

bool Packet::make(const int opcode, const std::string& json_data)
{
  return make(opcode, reinterpret_cast<const uint8_t*>(json_data.c_str()), json_data.size());
}

bool Packet::fill(const uint8_t* packet_byte_stream, const size_t byte_stream_size)
{
  if (packet_byte_stream == nullptr || byte_stream_size <= UNDEFINED_SIZE)
    return false;

  // Reset buffer
  RS_SAFE_DELETE_ARRAY(buffer_);

  // Allocate buffer
  buffer_      = new uint8_t[byte_stream_size];
  buffer_size_ = byte_stream_size;

  // Copy data
  std::memcpy(buffer_, packet_byte_stream, byte_stream_size);

  return true;
}

bool Packet::fill(const std::vector<uint8_t>& packet_byte_stream)
{
  return Packet::fill(packet_byte_stream.data(), packet_byte_stream.size());
}

bool Packet::validation() const
{
  if (buffer_ == nullptr || buffer_size_ <= UNDEFINED_SIZE)
    return false;

  if (buffer_size_ < HEADER_SIZE + TRAILER_SIZE)
    return false;

  // Check header
  if (auto header_ptr = this->header(); header_ptr == nullptr)
    return false;
  else
  {
    if (header_ptr->SOH_ != SOH || header_ptr->STX_ != STX)
      return false;

    // if (header_ptr->data.source <= UNDEFINED_TRANSPORT || header_ptr->data.destination <= UNDEFINED_TRANSPORT)
    //   return false;

    if (header_ptr->data.opcode <= UNDEFINED_OPCODE || header_ptr->data.payload_size <= UNDEFINED_SIZE)
      return false;

    if (header_ptr->data.total_size != buffer_size_)
      return false;
  }

  // Check trailer
  if (auto trailer_ptr = this->trailer(); trailer_ptr == nullptr)
    return false;
  else
  {
    if (trailer_ptr->ETX_ != ETX)
      return false;
  }

  return true;
}

void Packet::updateVersion(uint16_t version) const
{
  if (buffer_ == nullptr)
    return;

  std::memcpy(buffer_ + OFFSET_VERSION, &version, sizeof(uint16_t));
}

void Packet::updateSource(uint8_t source) const
{
  if (buffer_ == nullptr)
    return;

  std::memcpy(buffer_ + OFFSET_SOURCE, &source, sizeof(uint8_t));
}

void Packet::updateDestination(uint8_t destination) const
{
  if (buffer_ == nullptr)
    return;

  std::memcpy(buffer_ + OFFSET_DESTINATION, &destination, sizeof(uint8_t));
}

void Packet::updateTimestamp(uint64_t microsec) const
{
  if (buffer_ == nullptr)
    return;

  std::memcpy(buffer_ + OFFSET_TIMESTAMP, &microsec, sizeof(uint64_t));
}

}  // namespace rs
