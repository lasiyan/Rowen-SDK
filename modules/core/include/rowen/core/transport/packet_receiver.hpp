#pragma once

#include <unistd.h>

#include <functional>
#include <mutex>
#include <rowen/core/transport/packet_typedef.hpp>
#include <vector>

namespace rs {

class PacketReceiver
{
  using OnReceivedCallback = std::function<void(const uint8_t*, const ssize_t)>;

 public:
  using Result = std::vector<std::vector<uint8_t>>;

 public:
  PacketReceiver()                                 = default;
  PacketReceiver(const PacketReceiver&)            = delete;
  PacketReceiver& operator=(const PacketReceiver&) = delete;
  ~PacketReceiver()                                = default;

 public:
  void store(const uint8_t* buffer, const ssize_t size);

  Result grab();

  size_t grab(Result& result);

  void clear();

  void attachCallback(const OnReceivedCallback& callback);

  void resetCallback();

  decltype(auto) cache_size() const { return cache_receive_buffer_.size(); }
  decltype(auto) buffer_size() const { return received_packet_buffer_.size(); }
  decltype(auto) using_callback() const { return received_packet_callback_ != nullptr; }

 private:
  void progress();

 public:
 private:
  // Receive
  std::vector<uint8_t> cache_receive_buffer_ = {};
  rs::Packet::Header*  progress_header_      = nullptr;
  std::mutex           progress_mutex_;

  // by. store
  std::mutex received_packet_locker_ = {};
  Result     received_packet_buffer_ = {};

  // by. callback
  std::mutex         received_packet_callback_locker_ = {};
  OnReceivedCallback received_packet_callback_        = nullptr;
};

};  // namespace rs
