#include <functional>
#include <iostream>
#include <rowen/ipc/domain/listener.hpp>
#include <thread>

class StreamServer
{
 public:
  void onConnected(const rs::ipc::domain::listener::Client* client)
  {
    std::cout << "Client connected : socket " << client->id() << std::endl;
  }

  void onDisconnected(const rs::ipc::domain::listener::Client* client)
  {
    std::cout << "Client disconnected : socket " << client->id() << std::endl;
  }

  void onReceived(const rs::ipc::domain::listener::Client* client, const uint8_t* data, int size)
  {
    std::cout << "Received data from client :  socket " << client->id() << std::endl;
    std::cout << "  Size: " << size << std::endl;

    // echo
    auto res = listener_.send(client, data, size, 0);
    if (res == false)
    {
      std::cout << "Failed to send data" << std::endl;
      std::cout << listener_.error() << std::endl;
    }
  }

  rs::ipc::domain::listener listener_;
};

inline int run_uds_server()
{
  StreamServer server;

  server.listener_.attachConnectedCallback(std::bind(&StreamServer::onConnected, &server, std::placeholders::_1));
  server.listener_.attachDisconnectedCallback(std::bind(&StreamServer::onDisconnected, &server, std::placeholders::_1));
  server.listener_.attachReceivedCallback(std::bind(&StreamServer::onReceived, &server, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));

  auto domain = "/tmp/example_domain.sock";
  auto res    = server.listener_.running(domain, { .client_buffer_size = 999 });

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
      // send data
      auto res = client.send(reinterpret_cast<const uint8_t*>("BroadCast.."), 14);
      if (res == false)
        printf("%d : broadcast err : %s\n", client.id(), client.error().c_str());
    }
  }

  server.listener_.stop();

  return 0;
}
