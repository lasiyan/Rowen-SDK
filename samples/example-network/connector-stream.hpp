#include <iostream>
#include <thread>

#include "rowen/network/connector_stream.hpp"

inline int run_tcp_client()
{
  rs::network::stream_connector connector_;

  // try connection
  do
  {
    rs::network::stream_connector::argument args;
    args.send_buffer_size = 5;

    auto domain = "/tmp/example_domain.sock";
    auto res    = connector_.connect("192.168.0.241", 9999, args);

    if (res == false)
    {
      std::cout << "Failed to connect server" << std::endl;
      std::cout << connector_.error() << std::endl;
    }

    if (connector_.isConnected())
    {
      std::cout << "Connected to server" << std::endl;
      break;
    }
  } while (true);

  const char* send_data = "Hello, World";
  const auto  send_size = strlen(send_data);

  // -----------------------------------------------------------------------------
  // sending thread
  std::thread thread_send([&] {
    while (true)
    {
      std::this_thread::sleep_for(std::chrono::milliseconds(1000));

      auto data = std::make_unique<uint8_t[]>(send_size);
      memcpy(data.get(), send_data, send_size);

      auto res = connector_.send(data.get(), send_size);
      if (res != send_size)
      {
        std::cout << "Failed to send data : " << connector_.error() << std::endl;
      }
      else
      {
        std::cout << "Sent data to server" << std::endl;
      }
    }
  });

  // -----------------------------------------------------------------------------
  // receiving thread
  uint8_t buffer[4096]   = {};
  size_t  total_received = 0;

  while (true)
  {
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    auto received = connector_.recv(buffer, sizeof(buffer));

    if (received == 0)
    {
      std::cout << "Failed to receive data 0 : " << connector_.error() << std::endl;
      continue;
    }
    else if (received < 0)
    {
      std::cout << "Failed to receive data -1 : " << connector_.error() << std::endl;
      continue;
    }

    memcpy(buffer + total_received, buffer, received);
    total_received += received;

    // only working on echo server
    if (total_received >= send_size)
    {
      std::cout << "Received : " << buffer << std::endl;
      total_received = 0;
      memset(buffer, 0, sizeof(buffer));
    }
    else
    {
      std::cout << "Received : " << total_received << " bytes" << std::endl;
    }
  }

  return 0;
}
