#ifndef RESOLVER_HPP_J1BOIY8G
#define RESOLVER_HPP_J1BOIY8G

#include "request.hpp"
#include "error.hpp"
#include "loop.hpp"

namespace uvpp
{

inline error read_addr_info(int status, addrinfo* res, char* buf, bool &is_ip4)
{
  if(status == 0){
    if(res->ai_family == AF_INET){
      uv_ip4_name(reinterpret_cast<struct sockaddr_in*>(res->ai_addr), buf, res->ai_addrlen);
      is_ip4 = true;
    } else if(res->ai_family == AF_INET6) {
      uv_ip6_name(reinterpret_cast<struct sockaddr_in6*>(res->ai_addr), buf, res->ai_addrlen);
      is_ip4 = false;
    } else {
      is_ip4 = false;
      return error(EAI_ADDRFAMILY);
    }
  }
  return error(status);
}

class Resolver : public request<uv_getaddrinfo_t>
{
public:
  typedef std::function<void(error err, bool is_ip4, const std::string& addr)> ResolveCallback;

  Resolver():
    request<uv_getaddrinfo_t>(), m_loop(uv_default_loop())
  {
  }

  Resolver(loop& l) :
    request<uv_getaddrinfo_t>(),
    m_loop(l.get())
  {
  }

  error resolve(const std::string& addr, ResolveCallback callback)
  {
    callbacks::store(get()->data, internal::uv_cid_resolve, callback);
    return error(uv_getaddrinfo(m_loop, get(),
                  [](uv_getaddrinfo_t* req, int status, struct addrinfo* res)
                  {
                    std::shared_ptr<addrinfo> res_holder(res, [](addrinfo* res)
                        {
                          uv_freeaddrinfo(res);
                        });
                    char addr[128] = {'\0'};
                    bool is_ip4;
                    error err = read_addr_info(status, res, addr, is_ip4);
                    callbacks::invoke<decltype(callback)>(req->data, internal::uv_cid_resolve, err, is_ip4, addr);
                  }, addr.c_str(), 0, 0));
  }

private:
  uv_loop_t *m_loop;
};
}/*end namespace uvpp*/

#endif /* end of include guard: RESOLVER_HPP_J1BOIY8G */
