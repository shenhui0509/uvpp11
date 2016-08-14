#ifndef LOOP_THREAD_POOL_HPP_TVCBWHP1
#define LOOP_THREAD_POOL_HPP_TVCBWHP1

/* the thread numbers is fixed, so use std::array instead of std::vector*/
#include <array>
#include <queue>
#include <memory>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <future>
#include <functional>

namespace uvpp
{
constexpr size_t default_thread_nums = 4;

template<size_t thread_nums = default_thread_nums>
class ThreadPool
{
public:

    ThreadPool();
    template<class F, class... Args>
    auto equeue(F &&f, Args &&... args)
    ->std::future<typename std::result_of<F(Args...)>::type>;
    ~ThreadPool();

private:
    std::array<std::thread, thread_nums> workers;

    std::queue< std::function<void()>> tasks;

    std::mutex queue_mutex;
    std::condition_variable m_cond;
    bool stop;
};

template<size_t thread_nums> ThreadPool<thread_nums>::ThreadPool():
  stop(false)
{
  for(size_t i = 0; i < thread_nums; ++i) {
    workers.emplace_back(
        [this]()
        {
          while(1) {
            std::function<void()> task;
            {
              std::unique_lock<std::mutex> lock(this->queue_mutex);
              this->m_cond.wait(lock, [this]{return this->stop || !this->tasks.empty();});
              if(this->stop && this->tasks.empty())
                return;
              task = std::move(this->tasks.front());
              this->tasks.pop();
            }
            task();
          }
        }
        );
  }
}

template<size_t thread_nums>
template<class F, class... Args>
auto ThreadPool<thread_nums>::equeue(F&& f, Args&&... args)
  ->std::future<typename std::result_of<F(Args...)>::type>
{
  using ReturnType = typename std::result_of<F(Args...)>::type;
  auto task = std::make_shared< std::packaged_task<ReturnType()> >(
        std::bind(std::forward<F>(f), std::forward<Args>(args)...)
      );
  std::future<ReturnType> res = task->get_future();
  {
    std::unique_lock<std::mutex> lock(queue_mutex);
    if(stop)
      throw std::runtime_error("enqueue into stopped ThreadPool");
    tasks.emplace([task](){(*task)();});
  }
  m_cond.notify_one();
  return res;
}

template<size_t thread_nums>
ThreadPool<thread_nums>::~ThreadPool()
{
  {
    std::unique_lock<std::mutex> lock(queue_mutex);
    stop = true;
  }
  m_cond.notify_one();
  for(auto& woker : workers)
    woker.join();
}
}//namespace uvpp

#endif /* end of include guard*/
