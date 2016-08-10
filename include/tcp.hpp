#ifndef TCP_HPP_JYMCD4ZO
#define TCP_HPP_JYMCD4ZO

#include "error.hpp"
#include "loop.hpp"
#include "net.hpp"
#include "stream.hpp"

namespace uvpp {

/*
 * Wrapper for uv_tcp_t
 * example

 * class TcpEchoServer
 * {
 * public:
 *   TcpEchoServer(int port):
 *     m_server(), m_cnt(0)
 *   {
 *     m_server.bind("0.0.0.0", port);
 *   }
 *
 *   void AfterRead(const char* buf, ssize_t len)
 *   {
 *     if(len < 0)
 *       return;
 *     std::cout << "After Read\n";
 *     m_clients[333]->write(buf, len, std::bind(&TcpEchoServer::AfterWrite, this,
 * std::placeholders::_1));
 *   }
 *
 *   void AfterWrite(uvpp::error err)
 *   {
 *     if(err)
 *       return;
 *     ++m_cnt;
 *     std::cout << m_cnt << "\n";
 *   }
 *
 *   void OnConnection(uvpp::error err)
 *   {
 *     if(err)
 *       return;
 *     std::cout << "On new connect\n";
 *     uvpp::Tcp* sp_client(new uvpp::Tcp);
 *     m_server.accept(*sp_client);
 *     m_clients.insert(std::pair<uint32_t, uvpp::Tcp*>(333, sp_client));
 *     sp_client->template read_start<0>(std::bind(&TcpEchoServer::AfterRead,
 * this, std::placeholders::_1, std::placeholders::_2));
 *   }
 *
 *   void StartListen()
 *   {
 *     m_server.listen(std::bind(&TcpEchoServer::OnConnection, this,
 * std::placeholders::_1));
 *   }
 *
 * private:
 *     uvpp::Tcp m_server;
 *     size_t    m_cnt;
 *     std::map<uint32_t, uvpp::Tcp*> m_clients;
 * };
 *
 */
class Tcp : public stream<uv_tcp_t> {
public:
  Tcp() : stream() { uv_tcp_init(uv_default_loop(), get()); }

  Tcp(loop &l) : stream() { uv_tcp_init(l.get(), get()); }

  error nodelay(bool enable) {
    return error(uv_tcp_nodelay(get(), enable ? 1 : 0));
  }

  error keepalive(bool enable, unsigned int delay) {
    return error(uv_tcp_keepalive(get(), enable ? 1 : 0, delay));
  }

  error simultaneous_accepts(bool enable) {
    return error(uv_tcp_simultaneous_accepts(get(), enable ? 1 : 0));
  }

  error bind(const std::string &ip, int port) {
    ip4_addr addr = to_ip4_addr(ip, port);
    return error(uv_tcp_bind(get(), reinterpret_cast<sockaddr *>(&addr), 0));
  }

  error bind6(const std::string &ip, int port) {
    ip6_addr addr = to_ip6_addr(ip, port);
    return error(uv_tcp_bind(get(), reinterpret_cast<sockaddr *>(&addr), 0));
  }

  error connect(const std::string &ip, int port, CallbackWithResult callback) {
    callbacks::store(get()->data, internal::uv_cid_connect, callback);
    ip4_addr addr = to_ip4_addr(ip, port);
    return error(uv_tcp_connect(
        new uv_connect_t, get(), reinterpret_cast<sockaddr *>(&addr),
        [](uv_connect_t *req, int status) /*connect call back*/
        {
          std::unique_ptr<uv_connect_t> req_holder(req);
          callbacks::invoke<decltype(callback)>(
              req->handle->data, internal::uv_cid_connect, error(status));
        }));
  }

  error connect6(const std::string &ip, int port, CallbackWithResult callback) {
    callbacks::store(get()->data, internal::uv_cid_connect6, callback);
    ip6_addr addr = to_ip6_addr(ip, port);
    return error(uv_tcp_connect(
        new uv_connect_t, get(), reinterpret_cast<sockaddr *>(&addr),
        [](uv_connect_t *req, int status) /*connect call back*/
        {
          std::unique_ptr<uv_connect_t> req_holder(req);
          callbacks::invoke<decltype(callback)>(
              req->handle->data, internal::uv_cid_connect6, error(status));
        }));
  }

  bool getsockname(bool &ip4, std::string &ip, int &port) {
    struct sockaddr_storage addr;
    int len = sizeof addr;
    if (uv_tcp_getsockname(get(), reinterpret_cast<struct sockaddr *>(&addr),
                           &len) == 0) {
      ip4 = (addr.ss_family == AF_INET);
      if (ip4)
        return from_ip4_addr(reinterpret_cast<ip4_addr *>(&addr), ip, port);
      else
        return from_ip6_addr(reinterpret_cast<ip6_addr *>(&addr), ip, port);
    }
    return false;
  }

  bool getpeername(bool &ip4, std::string &ip, int &port) {
    struct sockaddr_storage addr;
    int len = sizeof addr;
    if (uv_tcp_getpeername(get(), reinterpret_cast<struct sockaddr *>(&addr),
                           &len) == 0) {
      ip4 = (addr.ss_family == AF_INET);
      if (ip4)
        return from_ip4_addr(reinterpret_cast<ip4_addr *>(&addr), ip, port);
      else
        return from_ip6_addr(reinterpret_cast<ip6_addr *>(&addr), ip, port);
    }
    return false;
  }
};
} /* end namespace uvpp*/

#endif /* end of include guard: TCP_HPP_JYMCD4ZO */
