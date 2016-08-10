#ifndef FILE_HPP_JNXIGQY5
#define FILE_HPP_JNXIGQY5

#include <memory>
#include <chrono>
#include <iostream>
#include <list>

#include "error.hpp"
#include "request.hpp"
#include "loop.hpp"

namespace uvpp
{

  struct Stats {
    uint64_t dev = 0;
    uint64_t mode = 0;
    uint64_t nlink = 0;
    uint64_t uid = 0;
    uint64_t gid = 0;
    uint64_t rdev = 0;

    double size = 0;
    double ino = 0;
    double atime = 0;
    double mtime = 0;
    double ctime = 0;
  };

#define STAT_GET_DBL(name) \
  (static_cast<double>(s->st_##name))

#define STAT_GET_DATE(name) \
  (static_cast<double>(s->st_##name.tv_sec) * 1000 +\
   static_cast<double>(s->st_##name.tv_nsec / 1000000))

  inline Stats stats_from_uv(const uv_stat_t* s)
  {
    Stats stat;
    stat.dev   = s->st_dev;
    stat.mode  = s->st_mode;
    stat.nlink = s->st_nlink;
    stat.uid   = s->st_uid;
    stat.gid   = s->st_gid;
    stat.rdev  = s->st_rdev;
    stat.size  = STAT_GET_DBL(size);
    stat.ino   = STAT_GET_DBL(ino);
    stat.atime = STAT_GET_DATE(atim);
    stat.mtime = STAT_GET_DATE(mtim);
    stat.ctime = STAT_GET_DATE(ctim);

    return stat;
  }

  struct ReadOptions {
    int flags  = O_CREAT | O_RDWR;
    int mode   = S_IRUSR | S_IWUSR;
  };

  struct WriteOptions {
    int flags = O_WRONLY | O_RDWR;
    int mode  = S_IRUSR | S_IWUSR;
  };

  class File : public request<uv_fs_t>
  {
    public:
      class Entry
      {
        friend class File;
        public:
        const std::string& name()
        {
          return m_name;
        }

        const std::string& full_path()
        {
          return m_full_path;
        }

        uv_dirent_type_t type()
        {
          return m_ent.type;
        }

        const std::string type_string()
        {
          switch(m_ent.type) {
            case UV_DIRENT_FILE:
              return "FILE";

            case UV_DIRENT_DIR:
              return "DIR";

            case UV_DIRENT_LINK:
              return "LINK";

            case UV_DIRENT_FIFO:
              return "FIFO";

            case UV_DIRENT_SOCKET:
              return "SOCKET";

            case UV_DIRENT_CHAR:
              return "CHAR";

            case UV_DIRENT_BLOCK:
              return "CAHR";

            default:
              return "UNKNOWN";
          }
        }

        private:
        Entry(File* file, uv_dirent_t dir) :
          m_file(file), m_ent(dir)
        {
          m_name = dir.name;
          if(m_ent.type == UV_DIRENT_DIR) {
            m_full_path = m_file->m_path + m_name + "/";
          } else {
            m_full_path = m_file->m_path + m_name;
          }
        }

        File* m_file;
        uv_dirent_t m_ent;
        std::string m_name;
        std::string m_full_path;
      };

      File(const std::string& path):
        request<uv_fs_t>(),
        m_loop(uv_default_loop()),
        m_path(path)
    {}

      File(loop& l, const std::string& path):
        request<uv_fs_t>(),
        m_loop(l.get()),
        m_path(path)
    {}

      error open(int flags, int mode, CallbackWithResult callback)
      {
        auto open_callback = [this, callback](error err, uv_file file) {
          if(!err)
            this->m_file = file;
          callback(err);
        };

        callbacks::store(get()->data, internal::uv_cid_fs_open, open_callback);

        return error(uv_fs_open(m_loop.get(), get(), m_path.c_str(), flags, mode,
              [](uv_fs_t* req) {
              auto result = req->result;
              uv_fs_req_cleanup(req);
              callbacks::invoke<decltype(open_callback)>(req->data, internal::uv_cid_fs_open, error(result < 0 ? result : 0), result);
              }
              ));
  }


  error read(int64_t bytes, int64_t offset, std::function<void(const char *buf, ssize_t len)> callback)
  {
    if(!m_file)
      return error(UV_EIO);
    uv_buf_t buffer;
    buffer.base = new char[bytes];
    buffer.len  = bytes;

    auto read_callback = [this, callback, buffer](ssize_t result) {
      std::shared_ptr<char> base_holder(buffer.base, std::default_delete<char[]>());
      if(!result) {
        callback(nullptr, result);
      } else {
        callback(buffer.base, result);
      }
    };

    callbacks::store(get()->data, internal::uv_cid_fs_read, read_callback);

    return error(uv_fs_read(m_loop.get(), get(), m_file, &buffer, 1, offset,
          [](uv_fs_t* req) {
          auto result = req->result;
          uv_fs_req_cleanup(req);
          callbacks::invoke<decltype(read_callback)>(req->data, internal::uv_cid_fs_read, result);
          }));
  }

  error write(const char* buf, int len, int offset, CallbackWithResult callback)
  {
    if(!m_file)
      return error(UV_EIO);

    callbacks::store(get()->data, internal::uv_cid_fs_write, callback);
    uv_buf_t bufs[] = {uv_buf_t{const_cast<char*>(buf), static_cast<size_t>(len)}};

    return error(uv_fs_write(m_loop.get(), get(), m_file, bufs, 1, offset,
          [](uv_fs_t* req) {
          auto result = req->result;
          uv_fs_req_cleanup(req);
          callbacks::invoke<decltype(callback)>(req->data, internal::uv_cid_fs_write, error(result < 0 ? result : 0));
          }));
  }

  error close(Callback callback)
  {
    if(!m_file)
      return error(UV_EIO);
    
    callbacks::store(get()->data, internal::uv_cid_fs_close, callback);

    return error(uv_fs_close(m_loop.get(), get(), m_file, 
                [](uv_fs_t* req)
                {
                  uv_fs_req_cleanup(req);
                  callbacks::invoke<decltype(callback)>(req->data, internal::uv_cid_fs_close);
                }));
  }

  error close()
  {
    if(!m_file)
      return error(UV_EIO);

    return error(uv_fs_close(m_loop.get(), get(), m_file, nullptr));
  }

  error unlink(CallbackWithResult callback)
  {
    if(!m_file)
      return error(UV_EIO);

    callbacks::store(get()->data, internal::uv_cid_fs_unlink, callback);

    return error(uv_fs_close(m_loop.get(), get(), m_file, 
                [](uv_fs_t* req)
                {
                  int result = req->result;
                  uv_fs_req_cleanup(req);
                  callbacks::invoke<decltype(callback)>(req->data, internal::uv_cid_fs_unlink, error(result < 0 ? result : 0));
                }));
  }
  
  error unlink()
  {
    if(!m_file)
      return error(UV_EIO);

    return error(uv_fs_close(m_loop.get(), get(), m_file, nullptr));
  }

  error stats(std::function<void(error, Stats)> callback)
  {
    callbacks::store(get()->data, internal::uv_cid_fs_stats, callback);
    return error(uv_fs_stat(m_loop.get(), get(), m_path.c_str(),
                [](uv_fs_t* req)
                {
                  int result = req->result;
                  Stats stat;
                  
                  if(result >= 0){
                    auto s = static_cast<const uv_stat_t*>(req->ptr);
                    stat = stats_from_uv(s);
                  }

                  uv_fs_req_cleanup(req);
                  callbacks::invoke<decltype(callback)>(req->data, internal::uv_cid_fs_stats, error(result < 0 ? result : 0), stat);
                }));
  }

  Stats stats()
  {
    int err = uv_fs_stat(m_loop.get(), get(), m_path.c_str(), nullptr);
    
    if(err < 0){
      uv_fs_req_cleanup(get());
      throw exception(error(err).str());
    } else {
      Stats s = stats_from_uv(static_cast<const uv_stat_t*>(get()->ptr));
      uv_fs_req_cleanup(get());
      return s;
    }
  }
  
  error fsync(CallbackWithResult callback)
  {
    if(!m_file)
      return error(UV_EIO);

    callbacks::store(get()->data, internal::uv_cid_fs_fsync, callback);
    
    return error(uv_fs_fsync(m_loop.get(), get(), m_file,
                            [](uv_fs_t* req)
                            {
                              int result = req->result;
                              uv_fs_req_cleanup(req);
                              callbacks::invoke<decltype(callback)>(req->data, internal::uv_cid_fs_fsync, error(result < 0 ? result : 0));
                            }));
  }

  error rename(const std::string& new_name, CallbackWithResult callback)
  {
    if(m_file)
      return error(UV_EIO);

    callbacks::store(get()->data, internal::uv_cid_fs_rename, callback);

    return error(uv_fs_rename(m_loop.get(), get(), m_path.c_str(), new_name.c_str(),
                              [](uv_fs_t* req)
                              {
                                int result = req->result;
                                uv_fs_req_cleanup(req);
                                callbacks::invoke<decltype(callback)>(req->data, internal::uv_cid_fs_rename, error(result < 0 ? result : 0));
                              }));
  }

  error sendfile(const File &dst, int64_t in_offset, size_t length, CallbackWithResult callback)
  {
    if(!m_file || !dst.m_file)
      return error(UV_EIO);

    callbacks::store(get()->data, internal::uv_cid_fs_sendfile, callback);

    return error(uv_fs_sendfile(m_loop.get(), get(), dst.m_file, m_file, in_offset, length,
                [](uv_fs_t* req)
                {
                  int result = req->result;
                  uv_fs_req_cleanup(req);
                  callbacks::invoke<decltype(callback)>(req->data, internal::uv_cid_fs_sendfile, error(result < 0 ? result : 0));
                }));
  }

  error scandir(std::function<void(error, std::list<Entry>)> callback)
  {
    auto scandir_callback = [this, callback](int result)
    {
      std::list<Entry> files;
      if(result < 0){
        uv_fs_req_cleanup(this->get());
        callback(error(result), files);
      } else {
        uv_dirent_t ent;
        for(int i = 0; i < result; i++){
          error err = error(uv_fs_scandir_next(this->get(), &ent));
          if(!err){
            files.push_back(Entry(this, ent));
          }
        }
        uv_fs_req_cleanup(this->get());
        callback(error(result), files);
      }
    };

    callbacks::store(get()->data, internal::uv_cid_fs_scandir, scandir_callback);
    
    return error(uv_fs_scandir(m_loop.get(), get(), m_path.c_str(), 0,
                [](uv_fs_t* req)
                {
                  callbacks::invoke<decltype(scandir_callback)>(req->data, internal::uv_cid_fs_scandir, req->result);
                }));
  }

  std::list<Entry> scandir()
  {
    int err = uv_fs_scandir(m_loop.get(), get(), m_path.c_str(), 0, nullptr);
    if(err >= 0) {
      std::list<Entry> files;
      uv_dirent_t ent;
      int r;
      while( (r = uv_fs_scandir_next(get(), &ent)) != UV_EOF){
        files.push_back(Entry(this, ent));
      }
      uv_fs_req_cleanup(get());
      return files;
    } else {
      throw exception(error(err).str());
    }
  }



  private:
  loop m_loop;
  const std::string m_path;
  uv_file m_file = 0;
};

}/*end namespace uvpp*/

#endif /* end of include guard: FILE_HPP_JNXIGQY5 */
