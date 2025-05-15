#include "rowen/network/connector_stream.hpp"

#include <assert.h>
#include <fcntl.h>

#include "rowen/core/exception.hpp"

namespace rs {
namespace network {

const stream_connector::argument stream_connector::default_arguments_ = {};

void stream_connector::initialize(const std::string& server_address,
                                  const int          server_port,
                                  const argument&    args)
{
  server_ipv4_    = server_address;
  server_port_    = server_port;
  last_arguments_ = args;
}

bool stream_connector::connect(const std::string& server_address,
                               const int          server_port,
                               const argument&    args)
{
  server_ipv4_    = server_address;
  server_port_    = server_port;
  last_arguments_ = args;

  try
  {
    {
      std::lock_guard<std::mutex> locker(connector_locker_);

      // create socket
      if (socket_.open(SOCK_STREAM, args.socket_protocol) == false)
        throw rs::exception("open socket" + socket_.error());

      // set socket option : recv default flags
      socket_.setReceiveFlags(args.recv_flags);

      // set socket option : recv timeout
      if (socket_.setReceiveTimeout(args.recv_timeout) == false)
        throw rs::exception("set recv_timeout : " + socket_.error());

      // set socket option : send default flags
      socket_.setSendFlags(args.send_flags);

      // set socket option : send buffer size
      socket_.setSendBufferSize(args.send_buffer_size);

      // set socket option : send timeout
      if (socket_.setSendTimeout(args.send_timeout) == false)
        throw rs::exception("set send_timeout" + socket_.error());
    }

    // connection
    {
      long param = ::fcntl(socket_.id(), F_GETFL, 0);
      param |= O_NONBLOCK;
      ::fcntl(socket_.id(), F_SETFL, param);

      // connect
      fd_set fdset;
      FD_ZERO(&fdset);

      // timeout
      float timeout_sec = args.connect_timeout;
      if (timeout_sec < 0.01)
        timeout_sec = 0.5;

      if (socket_.connect(server_address, server_port) == false)
      {
        if (errno != EINPROGRESS)
          throw rs::exception("connect : " + socket_.error());

        do
        {
          std::lock_guard<std::mutex> locker(connector_locker_);

          FD_ZERO(&fdset);
          FD_SET(socket_.id(), &fdset);

          timeval timeout = float_to_timeval(timeout_sec);
          int     res     = ::select(socket_.id() + 1, NULL, &fdset, NULL, &timeout);

          if (res < 0 && errno != EINTR)
            throw rs::exception("connect : select : " + std::string(::strerror(errno)));

          if (res <= 0)
          {
            if (args.connect_timeout >= 0)
              throw rs::exception("connect : timeout : " + std::string(::strerror(errno)));
            else  // retry connection
            {
              error_ = "waiting for connection";
              usleep(100 * 1000);
              continue;
            }
          }

          int       option = 1;
          socklen_t len    = sizeof(option);

          res = ::getsockopt(socket_.id(), SOL_SOCKET, SO_ERROR, &option, &len);

          if (res < 0)
            throw rs::exception("connect : getsockopt : " + std::string(::strerror(errno)));

          if (option != 0)
            throw rs::exception("connect : delayed connection");

          break;
        } while (true);
      }

      param &= (~O_NONBLOCK);
      ::fcntl(socket_.id(), F_SETFL, param);
    }
  }
  catch (const rs::exception& e)
  {
    error_ = e.what();
    return false;
  }

  error_.clear();
  connected_ = true;

  return true;
}

void stream_connector::disconnect()
{
  std::lock_guard<std::mutex> locker(connector_locker_);

  connected_ = false;
  socket_.close();
}

ssize_t stream_connector::send(const uint8_t* data, size_t size, int send_flags, int retry_flag)
{
  if (connected_ == false && connect(server_ipv4_, server_port_, last_arguments_) == false)
    return -1;

  auto res = try_send(data, size, send_flags);

  // check retry flag
  bool retry = (retry_flag < 0) ? last_arguments_.send_retry : (retry_flag != 0);

  if (res == 0 && retry)  // 전송 실패 원인이 Connection 관련이라면 접속 후 재전송
  {
    assert(connected_ == false);

    if (connect(server_ipv4_, server_port_, last_arguments_))
      res = try_send(data, size, send_flags);
  }

  return res;
}

ssize_t stream_connector::send(const rs::Packet& packet, int send_flags, int retry_flag)
{
  packet.updateTimestamp();

  return send(packet.data(), packet.size(), send_flags, retry_flag);
}

ssize_t stream_connector::recv(uint8_t* data, size_t size, int recv_flags)
{
  if (connected_ == false)
  {
    error_ = "not connected to server";
    return 0;
  }

  auto res = socket_.recv(data, size, recv_flags);

  if (res == 0)
  {
    error_ = "connection closed : " + socket_.error();
    disconnect();
  }
  else if (res < 0)
  {
    if (errno == EAGAIN)
      error_ = "receive timeout";
    else
      error_ = socket_.error();
  }

  return res;
}

ssize_t stream_connector::sendToRecv(const uint8_t* send_data, const size_t send_size,
                                     uint8_t* recv_data, const size_t recv_size,
                                     int send_flags, int recv_flags)
{
  // 1. send 시도 (자동 reconnect 및 send_retry 포함)
  auto sent = this->send(send_data, send_size, send_flags, 1);

  if (sent <= 0)  // send 실패시 recv 불필요
    return sent;

  // 2. recv 시도 (recv_data가 nullptr인 경우 로컬 버퍼 사용)
  uint8_t temporary_recv_buffer[DEFAULT_RECV_BUFFER_SIZE] = {};

  uint8_t*   recv_data_ptr = recv_data ? recv_data : temporary_recv_buffer;
  const auto recv_size_ptr = recv_data ? recv_size : DEFAULT_RECV_BUFFER_SIZE;

  auto received = this->recv(recv_data_ptr, recv_size_ptr, recv_flags);

  // recv 성공 또는 연결에 의한 recv 실패시
  if (received > 0 || received == 0)
    return received;

  // --- recv 실패 처리 ---
  const bool is_temporary_error  = (errno == EAGAIN || errno == EWOULDBLOCK || errno == EINTR);
  const bool receive_is_blocking = (last_arguments_.recv_timeout <= 0);

  bool retry_receive = false;

  if (is_temporary_error)
    retry_receive = true;

  if (last_arguments_.recv_retry == true && receive_is_blocking == false)
    retry_receive = true;

  if (retry_receive)
  {
    received = this->recv(recv_data_ptr, recv_size_ptr, recv_flags);

    if (received > 0 || received == 0)  // recv 성공 또는 연결에 의한 recv 실패시 중단.
      return received;
  }

  assert(received < 0);
  return received;
}

ssize_t stream_connector::try_send(const uint8_t* data, size_t size, int send_flags)
{
  connector_locker_.lock();
  auto res = socket_.send(data, size, send_flags);
  connector_locker_.unlock();

  if (res <= 0)
    error_ = socket_.error();

  if (res == 0)
    disconnect();

  return res;
}

};  // namespace network
};  // namespace rs
