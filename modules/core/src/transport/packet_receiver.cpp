#include <algorithm>
#include <rowen/core/transport/packet_receiver.hpp>

namespace rs {

void PacketReceiver::store(const uint8_t* buffer, const ssize_t size)
{
  std::unique_lock<std::mutex> locker(progress_mutex_);
  cache_receive_buffer_.insert(cache_receive_buffer_.end(), buffer, buffer + size);

  progress();
}

PacketReceiver::Result PacketReceiver::grab()
{
  std::lock_guard<std::mutex> locker(received_packet_locker_);
  Result                      result = received_packet_buffer_;
  received_packet_buffer_.clear();
  return result;
}

size_t PacketReceiver::grab(Result& result)
{
  std::lock_guard<std::mutex> locker(received_packet_locker_);
  result.swap(received_packet_buffer_);
  received_packet_buffer_.clear();
  return result.size();
}

void PacketReceiver::clear()
{
  {
    std::lock_guard<std::mutex> locker(received_packet_locker_);
    received_packet_buffer_.clear();
  }

  {
    std::lock_guard<std::mutex> locker(progress_mutex_);
    cache_receive_buffer_.clear();
  }
}

void PacketReceiver::attachCallback(const OnReceivedCallback& callback)
{
  received_packet_callback_ = callback;
}

void PacketReceiver::resetCallback()
{
  received_packet_callback_ = nullptr;
}

void PacketReceiver::progress()
{
  while (true)
  {
    try
    {
      // Step 1. Find Packet Header
      if (progress_header_ == nullptr)
      {
        auto iter = cache_receive_buffer_.begin();

        do
        {
          // Find SOH
          iter = std::find(iter, cache_receive_buffer_.end(), Packet::SOH);

          // If not found, wait for next data...
          if (iter == cache_receive_buffer_.end())
            break;

          // If not enough data, wait for next data...
          if (iter + rs::Packet::HEADER_SIZE > cache_receive_buffer_.end())
            break;

          // Check candidate header
          auto header_candidate = reinterpret_cast<rs::Packet::Header*>(&(*iter));
          if (header_candidate == nullptr)
            break;

          // Check candidate header validation
          if (header_candidate->STX_ != rs::Packet::STX)
          {
            iter++;
            continue;
          }

          if (header_candidate->data.opcode < 0 || header_candidate->data.payload_size < 0)
          {
            iter++;
            continue;
          }

          if (header_candidate->data.total_size != rs::Packet::PACKET_SIZE(header_candidate->data.payload_size))
          {
            iter++;
            continue;
          }

          // Find Packet !
          // erase previous data to make SOH to first byte and make header to first byte
          {
            iter             = cache_receive_buffer_.erase(cache_receive_buffer_.begin(), iter);
            progress_header_ = reinterpret_cast<rs::Packet::Header*>(&(*iter));
            break;
          }
        } while (iter < cache_receive_buffer_.end());
      }

      // Assert Packet Header
      if (progress_header_ == nullptr)
        break;

      // Step 2. Receive Packet Data until ETX
      if (progress_header_ != nullptr)
      {
        rs::Packet::Header* packet_header = static_cast<rs::Packet::Header*>(progress_header_);

        auto total_packet_size = packet_header->data.total_size;

        // Check receive buffer size is enough to process
        if (cache_receive_buffer_.size() < total_packet_size)
          break;

        // Check ETX
        auto trailer = reinterpret_cast<const rs::Packet::Trailer*>(cache_receive_buffer_.data() + rs::Packet::HEADER_SIZE + packet_header->data.payload_size);
        if (trailer == nullptr || trailer->ETX_ != rs::Packet::ETX)
          throw total_packet_size;

        // Get data and return
        if (received_packet_callback_)
        {
          received_packet_callback_(cache_receive_buffer_.data(), total_packet_size);
        }
        else
        {
          std::lock_guard<std::mutex> locker(received_packet_locker_);
          received_packet_buffer_.emplace_back(cache_receive_buffer_.begin(), cache_receive_buffer_.begin() + total_packet_size);
        }

        // Remove processed data
        {
          cache_receive_buffer_.erase(cache_receive_buffer_.begin(), cache_receive_buffer_.begin() + total_packet_size);
          progress_header_ = nullptr;
        }
      }
    }
    catch (const decltype(Packet::Header::Data::total_size) drop_size)
    {
      // Drop
      cache_receive_buffer_.erase(cache_receive_buffer_.begin(), cache_receive_buffer_.begin() + drop_size);
      progress_header_ = nullptr;
      break;
    }
    catch (...)
    {
      // Drop
      progress_header_ = nullptr;
      break;
    }

    usleep(1);
  }
}

}  // namespace rs
