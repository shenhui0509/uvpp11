#ifndef ASYNC_HPP_CVBQ2K5U
#define ASYNC_HPP_CVBQ2K5U

#include "error.hpp"
#include "handle.hpp"
#include "uvpp_defs.hpp"
#include "loop.hpp"

namespace uvpp
{

class Async : public handle<uv_async_t>
{
public:
  Async(loop &l) :
    handle<uv_async_t>(),
    m_loop(l.get())
  {
  }

  Async() :
    handle<uv_async_t>(),
    m_loop(uv_default_loop())
  {
  }

  error init(Callback callback)
  {
    callbacks::store(get()->data, internal::uv_cid_async, callback);
    return error(uv_async_init(m_loop, get(), [](uv_async_t* handle) {
      callbacks::invoke<decltype(callback)>(handle->data, internal::uv_cid_async);
    }));

  }
  error send()
  {
    return error(uv_async_send(get()));
  }
private:
  uv_loop_t *m_loop;
};
}/* ending namespace uvpp*/

#endif /* end of include guard: ASYNC_HPP_CVBQ2K5U */
