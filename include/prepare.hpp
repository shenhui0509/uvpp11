#ifndef PREPARE_HPP_KQOWFCFT
#define PREPARE_HPP_KQOWFCFT

#include "handle.hpp"
#include "error.hpp"
#include "loop.hpp"

namespace uvpp
{
/* Prepare handles will run the given callback once per loop iteration, 
 * right before polling for I/O
 * Some prepare work can be here
 * e.g. recording the time for per loop
 * class preparer
 * {
 * public:
 *  void record()
 *  {
 *    auto pre_worker = [this]()
 *    {
 *      this->last = this->cur;
 *      this->cur = std::chrono::system_clock::now();
 *    };
 *    err = _pre.start(pre_worker);
 *    if(err)
 *      throw execption(err.str());
 *  }
 *
 *  void stop_record()
 *  {
 *    err = _pre.stop();
 *    if(err)
 *      throw execption(err.str());
 *  }
 * private:
 *  std::chrono::time_point<std::chorono::system_clock> last, cur;
 *  Prepare _pre;
 * }
 */ 

class Prepare : public handle<uv_prepare_t>
{
public:
  Prepare()
  {
    uv_prepare_init(uv_default_loop(), get());
  }

  Prepare(loop& l)
  {
    uv_prepare_init(l.get(), get());
  }

  error start(Callback callback)
  {
    callbacks::store(get()->data, internal::uv_cid_prepare, callback);

    return error(uv_prepare_start(get(), 
                [](uv_prepare_t* handle)
                {
                  callbacks::invoke<decltype(callback)>(handle->data, internal::uv_cid_prepare);
                }));
  }

  error stop()
  {
    return error(uv_prepare_stop(get()));
  }
};
}/*end namespace uvpp*/
#endif /* end of include guard: PREPARE_HPP_KQOWFCFT */
