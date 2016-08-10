#ifndef PROCESS_HPP_30YGDXNU
#define PROCESS_HPP_30YGDXNU

#include "handle.hpp"
#include "error.hpp"
#include "loop.hpp"

namespace uvpp
{

/*
 *struct uv_process_options_t {
 *  uv_exit_cb  cb; void (*uv_exit_cb)(uv_process_t*, int64_t, int)
 *  const char* file; [> the executable file path<]
 *  char** args;      [> args pass to the programm <]
 *  char** env;       [> environments <]
 *  const char* cwd;  [> current work directory <]
 *  unsigned int flags;
 *
 *  int stdio_count;  [> the size of the stdio arr <]
 *  uv_stdio_container_t* stdio; [> stdio's fd<]
 *  uv_uid_t uid;     [>user id<]
 *  uv_gid_t gid;     [>group id<]
 *};
 *
 */
/* struct uv_stdio_container_t
 * {
 *    uv_stdio_flags flags;
 *    union {
 *      uv_stream_t* stream;
 *      int fd;
 *    } data;
 *
 * };
 *
 * struct uv_process_flags
 * {
 *  UV_PROCESS_SETUID = (1 << 0);
 *  UV_PROCESS_SETGID = (1 << 1);
 *  UV_PROCESS_WINDOWS_VERBATIM_ARGUMENTS = (1 << 2);
 *  UV_PROCESS_DETACHED = (1 << 3);
 *  UV_PROCESS_WINDOWS_HIDE = (1 << 4);
 * };
 */

class Process : public handle<uv_process_t>
{
public:
  
  typedef std::function<void(uv_process_t*, int64_t, int)> ExitCallback;

  Process(loop& l, uv_process_options_t opt):
    m_loop(l.get()), m_options(opt)
  {}

  Process(uv_process_options_t opt):
  m_loop(uv_default_loop()), m_options(opt)
  {}

  error spawn(ExitCallback callback)
  {
    callbacks::store(get()->data, internal::uv_cid_process, callback);
    m_options.exit_cb = [](uv_process_t* handle, int64_t exit_status, int term_signal)
    {
      callbacks::invoke<decltype(callback)>(handle->data, internal::uv_cid_process, handle, exit_status, term_signal);
    };

    return error(uv_spawn(m_loop, get(), &m_options));
  }

  static error kill(int pid, int signum)
  {
    return error(uv_kill(pid, signum));
  }

  static error process_kill(uv_process_t* handle, int signum)
  {
    return error(uv_process_kill(handle, signum));
  }
private:
  uv_loop_t* m_loop;
  uv_process_options_t m_options;
};


}/*end namespace uvpp*/

#endif /* end of include guard: PROCESS_HPP_30YGDXNU */
