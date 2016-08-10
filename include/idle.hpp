#ifndef IDLE_HPP_XI8LUKW1
#define IDLE_HPP_XI8LUKW1

#include "handle.hpp"
#include "error.hpp"
#include "loop.hpp"

namespace uvpp
{

class Idle : public handle<uv_idle_t>
{
public:
  Idle(Callback callback):
    handle<uv_idle_t>(),
    m_loop(uv_default_loop())
  {
    init(m_loop, callback);
  }

  Idle(loop& l, Callback callback) :
    handle<uv_idle_t>(),
    m_loop(l.get())
  {
    init(m_loop, callback);
  }

  error start()
  {
    return error(uv_idle_start(get(),
                [](uv_idle_t* handle)
                {
                  callbacks::invoke<Callback>(handle->data, internal::uv_cid_idle);
                }  ));
  }

  error stop()
  {
    return error(uv_idle_stop(get()));
  }

private:
  void init(uv_loop_t* loop, Callback callback)
  {
    callbacks::store(get()->data, internal::uv_cid_idle, callback);
    uv_idle_init(loop, get());
  }

  uv_loop_t *m_loop;
};

}/*end namespace uvpp*/

#endif /* end of include guard: IDLE_HPP_XI8LUKW1 */
