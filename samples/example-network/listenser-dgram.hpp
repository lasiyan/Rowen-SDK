#include <functional>
#include <iostream>
#include <rowen/network/listener_dgram.hpp>
#include <thread>

class UDPServer
{
 public:
  void onReceiveData(const uint8_t* data, int size, const struct sockaddr* addr, socklen_t addr_len)
  {
    std::cout << "Received data from client : " << rs::network::dgram_listener::client_address(addr, addr_len) << std::endl;
    std::cout << "  Size: " << size << "  Data: " << data << std::endl;

    // echo
    auto res = listener_.send(data, size, *((struct sockaddr_in*)addr), addr_len);
    if (res == false)
    {
      std::cout << "Failed to send data" << std::endl;
      std::cout << listener_.error() << std::endl;
    }
  }

  rs::network::dgram_listener listener_;
};

inline int run_udp_server()
{
  UDPServer server;

  server.listener_.attachReceivedCallback(std::bind(&UDPServer::onReceiveData, &server, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4));

  rs::network::dgram_listener::argument args;
  args.listener_select_timeout   = -1;
  args.listener_send_buffer_size = 5;

  auto res = server.listener_.running(9999, args);

  if (res == false)
  {
    std::cout << "Failed to start server" << std::endl;
    std::cout << server.listener_.error() << std::endl;

    return -1;
  }

  while (true)
  {
    std::this_thread::sleep_for(std::chrono::seconds(1));

    std::cout << "Server is running..." << std::endl;
    if (server.listener_.error())
    {
      std::cout << "Error : " << server.listener_.error() << std::endl;
    }

    // if press 'q' key, then stop server
    if (std::cin.get() == 'q')
      break;
  }

  server.listener_.stop();

  return 0;
}
