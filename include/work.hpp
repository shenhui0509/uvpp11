#ifndef WORK_HPP_A0DKBE58
#define WORK_HPP_A0DKBE58

#include "request.hpp"
#include "error.hpp"
#include "loop.hpp"

namespace uvpp
{
/*
 * Wrapping the under layer thread pool for libuv
 * example
 * class worker
 * {
 * public:
 *  worker(int percent = 0) :
 *   m_percent(percent)
 *  {}
 *
 *  void StartWork
 *  {
 *    while(m_percent < 100){
 *      work w;
 *      w.queue_work(std::bind(&worker::task, this), std::bind(&worker::afterwork, this, std::placeholders::_1));
 *    }
 *  };
 *
 *  void task()
 *  {
 *    lock();
 *    m_percent += rand() % 10;
 *    unlock();
 *  }
 *  
 *  void afterwork()
 *  {
 *    cout << "conguraulations\n";
 *  }
 *private:
 *   int percent = 0;
 * };
 */
class Work : public request<uv_work_t>
{
  public:
    Work():
      request<uv_work_t>(),
      m_loop(uv_default_loop())
  {}

    Work(loop& l):
      request<uv_work_t>(),
      m_loop(l.get())
  {}

    error queue_work(Callback callback, CallbackWithResult afterwork)
    {
      callbacks::store(get()->data, internal::uv_cid_work, callback);
      callbacks::store(get()->data, internal::uv_cid_after_work, afterwork);

      return error(uv_queue_work(m_loop, get(),
                                [](uv_work_t* req)
                                {
                                  callbacks::invoke<decltype(callback)>(req->data, internal::uv_cid_work);
                                }, 
                                [](uv_work_t* req, int status)
                                {
                                  callbacks::invoke<decltype(afterwork)>(req->data, internal::uv_cid_after_work, error(status));
                                }));
    }
  private:
    uv_loop_t* m_loop;
};
}/*end namespace uvpp*/

#endif /* end of include guard: WORK_HPP_A0DKBE58 */
