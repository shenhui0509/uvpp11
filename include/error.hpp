#ifndef UVPP_ERROR_H_2U9E1XNY
#define UVPP_ERROR_H_2U9E1XNY

#include <assert.h>
#include <stdexcept>
#include <string>

#include <uv.h>

namespace uvpp
{

class exception : public std::runtime_error
{
public:
  exception(const std::string& err_msg) :
    std::runtime_error(err_msg) {}
};

class error
{
public:
  error(int c) : m_error(c) {}

public:
  explicit operator bool() const
  {
    return m_error != 0;
  }

  const char* c_str() const
  {
    return uv_strerror(m_error);
  }

  std::string str() const
  {
    return uv_strerror(m_error);
  }
private:
  int m_error;
};

} // ending namespace uvpp

#endif /* end of include guard: UVPP_ERROR_H_2U9E1XNY */
