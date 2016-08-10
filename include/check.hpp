#ifndef CHECK_HPP_MVR14RHS
#define CHECK_HPP_MVR14RHS

#include "handle.hpp"
#include "error.hpp"
#include "loop.hpp"

namespace uvpp
{
/* similar to Prepare, but callback is called after I/O polling*/
class Check : public handle<uv_check_t>
{
public:
  Check()
  {
    uv_check_init(uv_default_loop(), get());
  }

  Check(loop &l)
  {
    uv_check_init(l.get(), get());
  }

  error start(Callback callback)
  {
    callbacks::store(get()->data, internal::uv_cid_check, callback);
    return error(uv_check_start(get(),
                [](uv_check_t* handle)
                {
                  callbacks::invoke<decltype(callback)>(handle->data, internal::uv_cid_check);
                }));
  }

  error stop()
  {
    return error(uv_check_stop(get()));
  }
};
}/*end namespace uvpp*/

#endif /* end of include guard: CHECK_HPP_MVR14RHS */
