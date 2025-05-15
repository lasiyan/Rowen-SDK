#pragma once

#include <future>
#include <mutex>
#include <string>
#include <unordered_map>
#include <unordered_set>

#include "rowen/transport/packet_receiver.hpp"
#include "template/socket.hpp"

namespace rs {
namespace ipc {
namespace domain {

class listener
{
  using client_set = std::unordered_set<Socket, socket_hash, socket_equal>;

 public:
  using Client = rs::ipc::domain::Socket;

 public:
  struct argument
  {
    bool  reuse_domain            = true;
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
  virtual ~listener();

  /**
   * @brief running listener
   * @param domain_file : domain file path (ex. /tmp/test.sock)
   * @param arguments : listener arguments
   * @return true if success, otherwise false
   */
  bool running(const std::string& domain_path, const argument& arguments = default_arguments_);

  /**
   * @brief stop listener
   */
  void stop();

  /**
   * @brief send data to client
   * @param client : client instance (a.k.a. socket)
   * @param data : data buffer (with. unsigned char*)
   * @param size : data size (byte)
   * @param send_flag : send flag (default : listener argument client_send_flags)
   * @return send size
   */
  ssize_t send(const Client* client, const uint8_t* data, int size, int send_flag = -1);

  /**
   * @brief send data to client
   * @param client : client instance (a.k.a. socket)
   * @param packet : rs packet struct
   * @param send_flag : send flag (default : listener argument client_send_flags)
   * @return send size
   */
  ssize_t send(const Client* client, const rs::Packet& packet, int send_flag = -1);

  /**
   * @brief get listener root socket
   * @return Socket class instance
   */
  class Socket& socket() { return socket_; }

  /**
   * @brief get last error message
   * @return error message
   */
  const char* error() const { return error_.empty() ? nullptr : error_.c_str(); }

  /**
   * @brief get connected clients
   * @return connected clients
   */
  const client_set& clients() const;

  /**
   * @brief get connected client by socket fd
   * @param fd : socket file descriptor
   * @return connected client pointer
   */
  const Client* client(int fd) const;

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

  std::string  error_         = "";
  class Socket socket_        = {};
  std::string  domain_path_   = "";
  std::mutex   listener_lock_ = {};

  // listener
  std::future<void> receiver_;
  std::atomic_bool  receiver_stop_ = true;

  // clients
  client_set         connected_clients_;
  mutable std::mutex connected_clients_mutex_;

  // rs packet
  std::unordered_map<const Client*, std::unique_ptr<PacketReceiver>> rs_packet_receiver_;
};

};  // namespace domain
};  // namespace ipc
};  // namespace rs
