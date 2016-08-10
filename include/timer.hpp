#ifndef TIMER_HPP_LWUGMVQI
#define TIMER_HPP_LWUGMVQI

#include <chrono>

#include "handle.hpp"
#include "error.hpp"
#include "loop.hpp"

namespace uvpp
{

class Timer : public handle<uv_timer_t>
{
public:
  Timer():
    handle<uv_timer_t>()
  {
    uv_timer_init(uv_default_loop(), get());
  }
  Timer(loop& l) :
    handle<uv_timer_t>()
  {
    uv_timer_init(l.get(), get());
  }

  error start(Callback callback, const std::chrono::duration<uint64_t, std::milli> &timeout,
                                 const std::chrono::duration<uint64_t, std::milli> &repeat)
  {
    callbacks::store(get()->data, internal::uv_cid_timer, callback);
    return error(uv_timer_start(get(),
                                [](uv_timer_t* handle)
                                {
                                  callbacks::invoke<decltype(callback)>(handle->data, internal::uv_cid_timer);
                                },
                                timeout.count(),
                                repeat.count()
                                ));
  }

  error start(Callback callback, const std::chrono::duration<uint64_t, std::milli> &timeout)
  {
    callbacks::store(get()->data, internal::uv_cid_timer, callback);
    return error(uv_timer_start(get(),
                          [](uv_timer_t* handle)
                          {
                            callbacks::invoke<decltype(callback)>(handle->data, internal::uv_cid_timer);
                          },
                          timeout.count(),
                          0
                          ));
  }

  error stop()
  {
    return error(uv_timer_stop(get()));
  }

  error again()
  {
    return error(uv_timer_again(get()));
  }
};
}/*end namespace uvpp*/

#endif /* end of include guard: TIMER_HPP_LWUGMVQI */
