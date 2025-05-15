#include <iostream>

#include "rowen/network/connector_dgram.hpp"

inline int run_udp_client()
{
  rs::network::dgram_connector connector_;

  rs::network::dgram_connector::argument args;
  args.send_timeout = 1;
  args.recv_timeout = 1;

  const char* server_ip   = "127.0.0.1";
  const int   server_port = 9999;

  auto res = connector_.initialize(server_ip, server_port, args);

  if (res == false)
  {
    std::cout << "Failed to init client" << std::endl;
    std::cout << connector_.error() << std::endl;
  }
  else
  {
    std::cout << "initialize success" << std::endl;
  }

  sockaddr_in server_addr     = {};
  server_addr.sin_family      = AF_INET;
  server_addr.sin_addr.s_addr = inet_addr(server_ip);
  server_addr.sin_port        = htons(server_port);
  socklen_t server_addr_len   = sizeof(server_addr);

  // -----------------------------------------------------------------------------

  const auto send_test = [&] {
    auto res = connector_.sendto((const uint8_t*)"Hello, World", 12);
    if (res != 12)
      std::cout << "Failed to send data : " << connector_.error() << std::endl;
    else
      std::cout << "Sent data to server" << std::endl;
  };

  const auto send_test_raw = [&] {
    auto res = connector_.sendto((const uint8_t*)"Hello, World", 12, (const struct sockaddr*)&server_addr, server_addr_len);

    if (res != 12)
      std::cout << "Failed to send data : " << connector_.error() << std::endl;
    else
      std::cout << "Sent data to server" << std::endl;
  };

  const auto recv_test = [&] {
    uint8_t buffer[4096] = {};
    auto    res          = connector_.recvfrom(buffer, sizeof(buffer));

    if (res <= 0)
      std::cout << "Failed to receive data : " << connector_.error() << std::endl;
    else
      std::cout << "Received data from server : " << buffer << std::endl;
  };

  const auto recv_test_raw = [&] {
    uint8_t buffer[4096] = {};
    auto    res          = connector_.recvfrom(buffer, sizeof(buffer), (struct sockaddr*)&server_addr, &server_addr_len);

    if (res <= 0)
      std::cout << "Failed to receive data : " << connector_.error() << std::endl;
    else
      std::cout << "Received data from server : " << buffer << std::endl;
  };

  while (true)
  {
    // if press 'q' key, then stop client
    auto ch = std::cin.get();

    if (ch == 'q')
      break;
    else if (ch == 's')
    {
      send_test();
      recv_test();
    }
    else if (ch == 'r')
    {
      send_test_raw();
      recv_test_raw();
    }

    usleep(100);
  }

  return 0;
}
