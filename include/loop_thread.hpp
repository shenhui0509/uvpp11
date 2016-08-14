#ifndef LOOP_THREAD_HPP_JU1PDQUK
#define LOOP_THREAD_HPP_JU1PDQUK

#include <thread>
#include <mutex>
#include <condition_variable>
#include <functional>
#include <string>

#include "uvpp_defs.hpp"
#include "loop.hpp"

namespace uvpp
{

class loop;

class LoopThread
{
public:
    typedef std::function<void(loop *)> ThreadInitCallback;

    LoopThread(const ThreadInitCallback &m_init_callback = ThreadInitCallback()):
        m_thread(),
        m_loop(nullptr),
        m_exiting(false),
        m_mutex(),
        m_cond(),
        m_callback(m_init_callback) {
    }

    ~LoopThread() {
        std::lock_guard<std::mutex> guard(m_mutex);
        m_exiting = true;
        if (m_loop) {
            m_loop->stop();
            m_thread.join();
        }
    }

    loop* start_loop()
    {
      m_thread = std::thread(std::bind(&LoopThread::thread_func, this));
      {
        std::unique_lock<std::mutex> lock(m_mutex);
        while(m_loop == nullptr) {
          m_cond.wait(lock);
        }
      }
      return m_loop;
    }

private:
    void thread_func()
    {
      loop thread_loop;
      if(m_callback){
        m_callback(&thread_loop);
      }

      {
        std::lock_guard<std::mutex> guard(m_mutex);
        m_loop = &thread_loop;
        m_cond.notify_one();
      }
      thread_loop.run();
      m_loop = nullptr;
    }

    std::thread m_thread;
    loop* m_loop;
    bool m_exiting;
    std::mutex m_mutex;
    std::condition_variable m_cond;
    ThreadInitCallback m_callback;
};

}//namespace uvpp

#endif /* end of include guard: LOOP_THREAD_HPP_JU1PDQUK */
