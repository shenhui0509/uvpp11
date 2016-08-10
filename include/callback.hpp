#ifndef CALLBACK_HPP_EWP7MNYB
#define CALLBACK_HPP_EWP7MNYB

#include <vector>
#include <functional>
#include <memory>

#include "error.hpp"
#include "uvpp_defs.hpp"

namespace uvpp
{

typedef std::function<void()> Callback;
typedef std::function<void(error)> CallbackWithResult;

namespace internal
{
enum uv_callback_id {
  uv_cid_close = 0,
#define XX(name) uv_cid_##name,
  UVPP_CALLBACK_MAP(XX)
#undef XX
  uv_cid_max
};

class callback_object_base
{
public:
  callback_object_base(void* data) :
    m_data(data)
  {}

  callback_object_base(callback_object_base&&) = default;
  callback_object_base(const callback_object_base&) = default;
  callback_object_base& operator=(callback_object_base&&) = default;
  callback_object_base& operator=(const callback_object_base&) = default;

  virtual ~callback_object_base()
  {}

  void* get_data() const
  {
    return m_data;
  }

private:
  void* m_data;
};

template<class callback_t>
class callback_object : public callback_object_base
{
public:
  callback_object(const callback_t& callback, void* data = nullptr)
    : callback_object_base(data),
      m_callback(callback)
  {}

public:
  template<class... Args>
  typename std::result_of<callback_t(Args&&...)>::type invoke(Args&&... args)
  {
    return m_callback(std::forward<Args>(args)...);
  }

private:
  callback_t m_callback;
};

}/* namespace internal */

typedef std::unique_ptr<internal::callback_object_base> callback_object_pointer;

/* callbacks table for each uv_handle_t,
 * user can regist callbacks to it
 * the supported callbac supprots std::function and lambda,
 * binding and closure(lambda with capture list) are also supported
 * callback is register(copied) via callback_id
 */

class callbacks
{
public:
  callbacks()
    : m_slots(internal::uv_cid_max)
  {
  }

  callbacks(int max_callbacks)
    : m_slots(max_callbacks)
  {
  }
  
  /* regist callback_object to the callbacks map*/
  template<class callback_t>
  static void store(void* target/*handle*/, int cid/*callback id*/, const callback_t& callback, void* data = nullptr)
  {
    reinterpret_cast<callbacks *>(target)->m_slots[cid] = callback_object_pointer(
          new internal::callback_object<callback_t>(callback, data));
  }

  /* get the data of the callback_object */
  template<class callback_t>
  static void* get_data(void* target, int cid)
  {
    return reinterpret_cast<callbacks*>(target)->m_slots[cid]->get_data();
  }

  /* invoke the callback via callback id */
  template<class callback_t, class... Args>
  static typename std::result_of<callback_t(Args...)>::type
  invoke(void* target, int cid, Args... args)
  {
    auto x = dynamic_cast<internal::callback_object<callback_t> *>(reinterpret_cast<callbacks*>(target)->m_slots[cid].get());
    assert(x);
    return x->invoke(std::forward<Args>(args)...);
  }

private:
  std::vector<callback_object_pointer> m_slots;
};


} /* namespace uvpp */

#endif /* end of include guard: CALLBACK_HPP_EWP7MNYB */
