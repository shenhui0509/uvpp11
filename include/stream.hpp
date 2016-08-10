#ifndef STREAM_HPP_RJFJBXOK
#define STREAM_HPP_RJFJBXOK

#include <algorithm>
#include <memory>

#include "uvpp_defs.hpp"
#include "error.hpp"
#include "handle.hpp"

namespace uvpp
{

template<class HANDLE_T>
class stream : public handle<HANDLE_T>
{
protected:
  stream():
    handle<HANDLE_T>()
  {}

public:
  error listen(CallbackWithResult callback, int backlog = 128)
  {
    callbacks::store(handle<HANDLE_T>::get()->data, internal::uv_cid_listen, callback);
    return error(uv_listen(handle<HANDLE_T>::template get<uv_stream_t>(),
                           backlog,
    [](uv_stream_t* s, int status) {
      callbacks::invoke<decltype(callback)>(s->data, internal::uv_cid_listen, error(status));
    }
                                                         ));
  }

  error accept(stream& client)
  {
    return error(uv_accept(handle<HANDLE_T>::template get<uv_stream_t>(),
                           client.handle<HANDLE_T>::template get<uv_stream_t>()));
  }

  template<size_t max_alloc_size>
  error read_start(std::function<void(const char* buf, ssize_t len)> callback)
  {
    callbacks::store(handle<HANDLE_T>::get()->data, uvpp::internal::uv_cid_read_start, callback);
    return error(uv_read_start(handle<HANDLE_T>::template get<uv_stream_t>(),
    [](uv_handle_t*, size_t suggested_size, uv_buf_t* buf) { /*alloc_cb*/
      assert(buf);
      auto size = std::max(suggested_size, max_alloc_size);
      buf->base = new char[size];
      buf->len  = size;
    },
    [](uv_stream_t* s, ssize_t nread, const uv_buf_t* buf) { /*read_cb*/
      std::shared_ptr<char> buf_base_holder(buf->base, std::default_delete<char[]>());

      if(nread < 0) {
        callbacks::invoke<decltype(callback)>(s->data, uvpp::internal::uv_cid_read_start,
                                              nullptr, 0);
      } else if(nread >= 0) {
        callbacks::invoke<decltype(callback)>(s->data, uvpp::internal::uv_cid_read_start,
                                              buf->base, nread);
      }
    }));
  }

  error read_start(std::function<void(const char* buf, ssize_t len)> callback)
  {
    return read_start<0>(callback);
  }

  error read_stop()
  {
    return error(uv_read_stop(handle<HANDLE_T>::template get<uv_stream_t>()));
  }

  error write(const char* buf, int len, CallbackWithResult callback)
  {
    uv_buf_t bufs[] = { uv_buf_t { const_cast<char*>(buf), static_cast<size_t>(len) } };
    callbacks::store(handle<HANDLE_T>::get()->data, uvpp::internal::uv_cid_write, callback);
    return error(uv_write(new uv_write_t, handle<HANDLE_T>::template get<uv_stream_t>(), bufs, 1,
    [](uv_write_t* req, int status) { /* write callback*/
      std::unique_ptr<uv_write_t> req_holder(req);
      callbacks::invoke<decltype(callback)>(req->handle->data, uvpp::internal::uv_cid_write, error(status));
    }));
  }

  error write(const std::string& buf, CallbackWithResult callback)
  {
    uv_buf_t bufs[] = { uv_buf_t{ const_cast<char*>(buf.c_str()), buf.length() } };
    callbacks::store(handle<HANDLE_T>::get()->data, uvpp::internal::uv_cid_write, callback);
    return error(uv_write(new uv_write_t, handle<HANDLE_T>::template get<uv_stream_t>(), bufs, 1,
    [](uv_write_t* req, int status) {
      std::unique_ptr<uv_write_t> req_holder(req);
      callbacks::invoke<decltype(callback)>(req->handle->data, uvpp::internal::uv_cid_write, error(status));
    }));
  }

  error write(const std::vector<char>& buf, CallbackWithResult callback)
  {
    uv_buf_t bufs[] = { uv_buf_t{ const_cast<char*>(&buf[0]), buf.size() } };
    callbacks::store(handle<HANDLE_T>::get()->data, uvpp::internal::uv_cid_write, callback);
    return error(uv_write(new uv_write_t, handle<HANDLE_T>::template get<uv_stream_t>(), bufs, 1,
    [](uv_write_t* req, int status) {
      std::unique_ptr<uv_write_t> req_holder(req);
      callbacks::invoke<decltype(callback)>(req->handle->data, uvpp::internal::uv_cid_write, error(status));
    }));
  }

  error shutdown(CallbackWithResult callback)
  {
    callbacks::store(handle<HANDLE_T>::get()->data, uvpp::internal::uv_cid_shutdown, callback);
    return error(uv_shutdown(new uv_shutdown_t, handle<HANDLE_T>::template get<uv_stream_t>(),
    [](uv_shutdown_t *req, int status) {
      std::unique_ptr<uv_shutdown_t> req_holder(req);
      callbacks::invoke<decltype(callback)>(req->handle->data, uvpp::internal::uv_cid_shutdown, error(status));
    }));
  }
};

}/* ending namespace uvpp*/
#endif /* end of include guard: STREAM_HPP_RJFJBXOK */
