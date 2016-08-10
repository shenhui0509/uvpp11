#ifndef FSPOLL_HPP_XEILHDMA
#define FSPOLL_HPP_XEILHDMA

#include "handle.hpp"
#include "error.hpp"
#include "file.hpp"

namespace uvpp
{

class FsPoll : public handle<uv_fs_poll_t>
{
public:
  FsPoll():
    handle<uv_fs_poll_t>()
  {
    uv_fs_poll_init(uv_default_loop(), get());
  }

  FsPoll(loop& l) : 
    handle<uv_fs_poll_t>()
  {
    uv_fs_poll_init(l.get(), get());
  }

  error start(const std::string& path, unsigned int interval, 
              std::function<void(error err, int status, Stats prev, Stats current)> callback)
  {
    callbacks::store(get()->data, internal::uv_cid_fs_poll, callback);
    
    return error(uv_fs_poll_start(get(), 
                                  [](uv_fs_poll_t *handle, int status, const uv_stat_t* prev, const uv_stat_t* current)
                                  {
                                    Stats _prev, _current;
                                    if(status >= 0){
                                      _prev = stats_from_uv(prev);
                                      _current = stats_from_uv(current);
                                    }
                                    callbacks::invoke<decltype(callback)>(handle->data, internal::uv_cid_fs_poll, error(status < 0 ? status : 0),
                                        status, _prev, _current);
                                  }, path.c_str(), interval));
  }

  error stop()
  {
    return error(uv_fs_poll_stop(get()));
  }
};
}/*end namespace uvpp*/

#endif /* end of include guard: FSPOLL_HPP_XEILHDMA */
