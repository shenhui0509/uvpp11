#ifndef LOOP_H_2TAHUCGP
#define LOOP_H_2TAHUCGP
#include <functional>
#include <memory>

#include "error.hpp"

namespace uvpp
{

class loop
{
public:
  /*default ctor
   * @param use_default indicates to use uv_default_loop or create new loop
   */
  loop(bool use_default = false) :
    m_use_default(use_default),
    m_uv_loop(use_default ? uv_default_loop() :
              new uv_loop_t, [this](uv_loop_t *loop)
  {
    destroy(loop);
  })
  {
    if(!m_use_default && uv_loop_init(m_uv_loop.get())) {
      throw std::runtime_error("uv_loop_init error");
    }
  }

  /*
   *dctor
   */

  ~loop()
  {
    if(m_uv_loop.get()) {
      uv_loop_close(m_uv_loop.get());
    }
  }

  /*
   *Disallow copy ctor and copy assign
   */
  loop(const loop&) = delete;
  loop& operator=(const loop&) = delete;


  /*
   * move ctor and copy is allowed
   */
  loop(loop&& other) : m_uv_loop(std::forward<decltype(other.m_uv_loop)>(other.m_uv_loop))
  {}

  loop& operator= (loop&& other)
  {
    if(this != &other) {
      m_uv_loop = std::forward<decltype(other.m_uv_loop)>(other.m_uv_loop);
    }
    return *this;
  }

  /* get the uv_loop_t handler */
  uv_loop_t* get()
  {
    return m_uv_loop.get();
  }

  /* start loop, use the default macro*/
  error run()
  {
    return error(uv_run(m_uv_loop.get(), UV_RUN_DEFAULT));
  }

  /* polls for new events without blocking*/
  error run_once()
  {
    return error(uv_run(m_uv_loop.get(), UV_RUN_ONCE));
  }

  /* wrap for functions for uv_loop_t*/
  void update_time()
  {
    uv_update_time(m_uv_loop.get());
  }

  /* get now ticks, delegate to uv_now */
  int64_t now()
  {
    return uv_now(m_uv_loop.get());
  }

  void stop()
  {
    uv_stop(m_uv_loop.get());
  }

private:
  void destroy(uv_loop_t* loop) const
  {
    if(!m_use_default)
      delete loop;
  }

  typedef std::function<void(uv_loop_t*)> Deleter;

  bool m_use_default;
  std::unique_ptr<uv_loop_t, Deleter> m_uv_loop;
};

/* utils for start default loop */
inline int run()
{
  return uv_run(uv_default_loop(), UV_RUN_DEFAULT);
}

/*utils for run once
 * polls for new events without blocking for the default loop
 */
 inline int run_once()
{
  return uv_run(uv_default_loop(), UV_RUN_ONCE);
}

}//ending ns uvpp

#endif /* end of include guard: LOOP_H_2TAHUCGP */
