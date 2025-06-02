#pragma once

#include <rowen/core/transport/packet_receiver.hpp>
#include <rowen/core/transport/packet_typedef.hpp>
#include <rowen/network/detail/listener.hpp>
#include <unordered_map>

namespace rs {
namespace network {

class stream_listener : public template_listener
{
 public:
  typedef struct ConnectedClient
  {
    class Socket client_socket     = {};
    char         client_ipaddr[16] = {};
  } Client;

 public:
  struct argument
  {
    int   socket_protocol         = 0;
    bool  reuse_address           = true;
    bool  reuse_port              = true;
    int   backlog                 = 5;
    bool  using_rs_packet         = false;
    bool  trace_rs_packet         = false;
    float listener_select_timeout = 1;  // timeout is seconds
    int   listener_buffer_size    = DEFAULT_RECV_BUFFER_SIZE;
    int   listener_recv_timeout   = 0;

    // for client
    int   client_recv_flags   = MSG_NOSIGNAL;
    float client_recv_timeout = 0;  // timeout is seconds
    int   client_send_flags   = MSG_NOSIGNAL;
    float client_send_timeout = 1;  // timeout is seconds
    int   client_buffer_size  = DEFAULT_SEND_BUFFER_SIZE;
  };

  // callback
  using OnConnectedCallback    = std::function<void(const Client*)>;
  using OnDisconnectedCallback = std::function<void(const Client*)>;
  using OnReceivedCallback     = std::function<void(const Client*, const uint8_t*, int)>;

 public:
  /**
   * @brief running listener
   * @param port : port number
   * @param arguments : listener arguments
   * @return true if success, otherwise false
   */
  bool running(int port, const argument& arguments = default_arguments_);

  /**
   * @brief send listener
   * @param client : client instance (a.k.a. ConnectedClient)
   * @param data : data buffer (with. unsigned char*)
   * @param size : data size (byte)
   * @param send_flag : send flag (default : listener argument client_send_flags)
   */
  ssize_t send(const Client* client, const uint8_t* data, int size, int send_flag = -1);

  /**
   * @brief send listener
   * @param client : client instance (a.k.a. ConnectedClient)
   * @param data : data buffer (with. unsigned char*)
   * @param size : data size (byte)
   * @param send_flag : send flag (default : listener argument client_send_flags)
   */
  ssize_t send(const Client* client, const rs::Packet& packet, int send_flag = -1);

  /**
   * @brief get connected clients
   * @return connected clients
   */
  const std::unordered_map<int, Client>& clients() const;

  /**
   * @brief get connected client
   * @param client_socket : client socket
   * @return connected client pointer
   */
  const Client* client(int client_socket) const;

  /**
   * @brief disconnect client
   * @param client : client instance (a.k.a. socket)
   * @param how : shutdown flag (default : SHUT_RD)
   */
  void disconnect(const Client* client, const int how = SHUT_RD);

 public:
  /**
   * @brief attach listener callbacks
   * @param callback : listener callback funtion pointer
   */
  void attachConnectedCallback(const OnConnectedCallback& callback);
  void attachDisconnectedCallback(const OnDisconnectedCallback& callback);
  void attachReceivedCallback(const OnReceivedCallback& callback);

 private:
  void onReceiveMessage(const argument& attr);

 private:
  static const argument default_arguments_;

 private:
  OnConnectedCallback    callback_connected_    = nullptr;
  OnDisconnectedCallback callback_disconnected_ = nullptr;
  OnReceivedCallback     callback_received_     = nullptr;

 private:
  mutable std::mutex              connected_clients_mutex_;
  std::unordered_map<int, Client> connected_clients_;

  // rs packet
  std::unordered_map<const Client*, std::unique_ptr<PacketReceiver>> rs_packet_receiver_;
};

};  // namespace network
};  // namespace rs
