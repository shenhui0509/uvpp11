# UVPP11 : a C++11 wrapper for libuv
## Features:
- Support arbitary function for uv's callback(wraps in std::function)
- lambda, clojure, bind .etc.
- C++ style interfaces, make libuv easy to use

example codes:
```C++
#include <map>
#include <memory>
#include <functional>

#include "uvpp.hpp"

class TcpEchoServer
{
public:
  TcpEchoServer(int port):
    m_server(), m_cnt(0)
  {
    m_server.bind("0.0.0.0", port);
  }

  void AfterRead(const char* buf, ssize_t len)
  {
    if(len < 0)
      return;
    std::cout << "After Read\n";
    m_clients[333]->write(buf, len, std::bind(&TcpEchoServer::AfterWrite, this, std::placeholders::_1));
  }
  
  void AfterWrite(uvpp::error err)
  {
    if(err)
      return;
    ++m_cnt;
    std::cout << m_cnt << "\n";
  }

  void OnConnection(uvpp::error err)
  {
    if(err)
      return;
    std::cout << "On new connect\n";
    uvpp::Tcp* sp_client(new uvpp::Tcp);
    m_server.accept(*sp_client);
    m_clients.insert(std::pair<uint32_t, uvpp::Tcp*>(333, sp_client));
    sp_client->template read_start<0>(std::bind(&TcpEchoServer::AfterRead, this, std::placeholders::_1, std::placeholders::_2));
  }
  
  void StartListen()
  {
    m_server.listen(std::bind(&TcpEchoServer::OnConnection, this, std::placeholders::_1));
  }

private:
    uvpp::Tcp m_server;
    size_t    m_cnt;
    std::map<uint32_t, uvpp::Tcp*> m_clients;
};

int main(int argc, char *argv[])
{
  TcpEchoServer server(8765);
  server.StartListen();
  return uv_run(uv_default_loop(), UV_RUN_DEFAULT);
}

```