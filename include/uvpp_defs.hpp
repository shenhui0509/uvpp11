#ifndef UVPP_DEFS_HPP_ZSV1RDD7
#define UVPP_DEFS_HPP_ZSV1RDD7

#define UVPP_CALLBACK_MAP(XX)       \
  XX(listen)                        \
  XX(read_start)                    \
  XX(write)                         \
  XX(shutdown)                      \
  XX(connect)                       \
  XX(connect6)                      \
  XX(work)                          \
  XX(after_work)                    \
  XX(timer)                         \
  XX(poll)                          \
  XX(signal)                        \
  XX(async)                         \
  XX(idle)                          \
  XX(fs_open)                       \
  XX(fs_read)                       \
  XX(fs_write)                      \
  XX(fs_close)                      \
  XX(fs_unlink)                     \
  XX(fs_stats)                      \
  XX(fs_fsync)                      \
  XX(fs_rename)                     \
  XX(fs_sendfile)                   \
  XX(fs_poll)                       \
  XX(fs_event)                      \
  XX(fs_scandir)                    \
  XX(resolve)                       \
  XX(check)                         \
  XX(prepare)                       \
  XX(process)

#define UNREACHABLE()               \
  assert(0)

#define DISALLOW_COPY_AND_ASSIGN(type) \
  type(const type&) = delete;           \
  type& operator=(const type&) = delete
#endif /* end of include guard: UVPP_DEFS_HPP_ZSV1RDD7 */
