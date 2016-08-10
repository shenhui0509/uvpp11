#ifndef REQUEST_HPP_UZQVBLWZ
#define REQUEST_HPP_UZQVBLWZ

#include "uvpp_defs.hpp"
#include "callback.hpp"
#include "error.hpp"

namespace uvpp
{

namespace
{

template<class REQUEST_T>
inline void free_request(REQUEST_T **req)
{
  if(*req == nullptr)
    return;

  if((*req)->data) {
    delete reinterpret_cast<callbacks*>((*req)->data);
    (*req)->data = nullptr;
  }

  switch ((*req)->type) {

  case UV_WORK:
    delete reinterpret_cast<uv_work_t*>(*req);
    break;

  case UV_FS:
    delete reinterpret_cast<uv_fs_t*>(*req);
    break;

  case UV_GETADDRINFO:
    delete reinterpret_cast<uv_getaddrinfo_t*>(*req);
    break;

  default:
    UNREACHABLE();
  }
}

}/* ending unamed namesapce*/

/*Wrapper for uv_request_t, or derived as uv_work_t,
 * Resources are released on the closse call as mandated by libuv NOT on the dtor
 * */
template<class REQUEST_T>
class request
{
protected:
  request():
    m_uv_request(new REQUEST_T),
    m_will_close(false)
  {
    assert(m_uv_request);
    m_uv_request->data = new callbacks;
    assert(m_uv_request->data);
  }

  request(request&& other):
    m_uv_request(other.m_uv_request),
    m_will_close(other.m_will_close)
  {
    other.m_uv_request = nullptr;
    other.m_will_close = false;
  }

  request& operator=(request&& other)
  {
    if(this == &other)
      return *this;
    m_uv_request = other.m_uv_request;
    m_will_close = other.m_will_close;
    other.m_uv_request = nullptr;
    other.m_will_close = false;
    return *this;
  }

  request(const request&) = delete;
  request& operator=(const request&) = delete;

  virtual ~request()
  {
    if(!m_will_close)
      free_request(&m_uv_request);
  }

public:
  template<class T = REQUEST_T>
  T* get()
  {
    return reinterpret_cast<T*>(m_uv_request);
  }

  template<class T = REQUEST_T>
  const T* get() const
  {
    return reinterpret_cast<const T*>(m_uv_request);
  }
  
  error cancel()
  {
    return error(uv_cancel(reinterpret_cast<uv_req_t*>(get())));
  }

private:
  REQUEST_T* m_uv_request;
  bool m_will_close;
};
}/*ending namespace uvpp*/

#endif /* end of include guard: REQUEST_HPP_UZQVBLWZ */
