#include <functional>
#include <iostream>
#include <thread>

#include "rowen/network/listener_stream.hpp"

class TCPServer
{
 public:
  void onClientConnected(const rs::network::stream_listener::Client* client)
  {
    std::cout << "Client connected : IP " << client->client_ipaddr << std::endl;
  }

  void onClientDisconnected(const rs::network::stream_listener::Client* client)
  {
    std::cout << "Client disconnected : IP " << client->client_ipaddr << std::endl;
  }

  void onReceiveData(const rs::network::stream_listener::Client* client, const uint8_t* data, int size)
  {
    std::cout << "Received data from client : " << client->client_ipaddr << std::endl;
    std::cout << "  Size: " << size << "  Data: " << data << std::endl;

    // echo
    auto res = listener_.send(client, data, size, 0);
    if (res == false)
    {
      std::cout << "Failed to send data" << std::endl;
      std::cout << listener_.error() << std::endl;
    }
  }

  rs::network::stream_listener listener_;
};

inline int run_tcp_server()
{
  TCPServer server;

  server.listener_.attachConnectedCallback(std::bind(&TCPServer::onClientConnected, &server, std::placeholders::_1));
  server.listener_.attachDisconnectedCallback(std::bind(&TCPServer::onClientDisconnected, &server, std::placeholders::_1));
  server.listener_.attachReceivedCallback(std::bind(&TCPServer::onReceiveData, &server, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));

  rs::network::stream_listener::argument args;
  args.listener_buffer_size    = 5;
  args.listener_select_timeout = 1.235;

  auto res = server.listener_.running(9999, args);

  if (res == false)
  {
    std::cout << "Failed to start server" << std::endl;
    std::cout << server.listener_.error() << std::endl;

    return -1;
  }

  while (true)
  {
    std::this_thread::sleep_for(std::chrono::milliseconds(10));

    std::cout << "Server is running..." << std::endl;
    if (server.listener_.error())
    {
      std::cout << "Error : " << server.listener_.error() << std::endl;
    }

    // if press 'q' key, then stop server
    if (std::cin.get() == 'q')
      break;

    // check client
    std::cout << "Connected client count : " << server.listener_.clients().size() << std::endl;
    for (const auto& client : server.listener_.clients())
    {
      auto clnt_sock = client.second.client_socket;

      // send data
      auto res = clnt_sock.send(reinterpret_cast<const uint8_t*>("BroadCast.."), 14);
      if (res == false)
        printf("broadcast err : to %s\n", client.second.client_ipaddr);
    }
  }

  server.listener_.stop();

  return 0;
}
