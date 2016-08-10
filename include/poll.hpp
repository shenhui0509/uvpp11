#ifndef POLL_HPP_MELFTG9V
#define POLL_HPP_MELFTG9V

#include "handle.hpp"
#include "error.hpp"
#include "loop.hpp"

namespace uvpp
{

class Poll : public handle<uv_poll_t>
{
public:
  Poll(int fd) :
    handle<uv_poll_t>()
  {
    uv_poll_init(uv_default_loop(), get(), fd);
  }

  Poll(loop& l, int fd) :
    handle<uv_poll_t>()
  {
    uv_poll_init(l.get(), get(), fd);
  }

  error start(int events,
              std::function<void(int status, int events)> callback)
  {
    callbacks::store(get()->data, internal::uv_cid_poll, callback);
    return error(uv_poll_start(get(), events, 
                [](uv_poll_t* handle, int status, int events)
                {
                  callbacks::invoke<decltype(callback)>(handle->data, internal::uv_cid_poll, status, events);
                }));
  }

  error stop()
  {
    return error(uv_poll_stop(get()));
  }
};
}/*end namespace uvpp*/

#endif /* end of include guard: POLL_HPP_MELFTG9V */
