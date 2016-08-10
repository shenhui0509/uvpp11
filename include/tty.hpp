#ifndef TTY_HPP_UOBFBZEL
#define TTY_HPP_UOBFBZEL

#include "error.hpp"
#include "stream.hpp"
#include "loop.hpp"
namespace uvpp
{

class TTY : public stream<uv_tty_t>
{
public:
  enum Type {
    STDIN,
    STDOUT,
    STDERR
  };

  TTY(Type type, bool readable):
    stream<uv_tty_t>(), m_type(type)
  {
    uv_tty_init(uv_default_loop(), get(), static_cast<int>(m_type), static_cast<int>(readable));
  }

  TTY(loop &l, Type type, bool readable) :
    stream<uv_tty_t>(), m_type(type)
  {
    uv_tty_init(l.get(), get(), static_cast<int>(m_type), static_cast<int>(readable));
  }

private:
  Type m_type;
};

}/*(end namespace uvpp*/

#endif /* end of include guard: TTY_HPP_UOBFBZEL */
