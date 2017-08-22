
#if ! defined __RUNTIME_TRANSPORT_AUTO_REGISTRY__
#define __RUNTIME_TRANSPORT_AUTO_REGISTRY__

#include "common.h"
#include "envelope.h"
#include "message.h"
#include "registry.h"

#include <vector>
#include <memory>

namespace runtime { namespace auto_registry {

using auto_active_t = active_function_t;
using auto_active_container_t = std::vector<auto_active_t>;
using auto_handler_t = int32_t;
using handler_manager_t = runtime::HandlerManager;

template <typename = void>
auto_active_container_t&
get_auto_registry();

template <typename ActiveFnT>
struct Registrar {
  auto_handler_t index;

  Registrar();
};

active_function_t
get_auto_handler(ShortMessage* const msg);

template <typename MessageT, action_any_function_t<MessageT>* f>
handler_t
make_auto_handler(MessageT* const msg);

template <typename ActiveFnT>
struct RegistrarWrapper {
  Registrar<ActiveFnT> registrar;
};

template <typename ActiveFnT>
auto_handler_t
register_active_fn();

template <typename F, F* f>
struct FunctorAdapter {
  using function_ptr_type = F;

  static constexpr F* get_function() { return f; }

  template <typename... A>
  auto operator()(A&&... a) -> decltype(f(std::forward<A>(a)...)) {
    return f(std::forward<A>(a)...);
   }
};

template <typename Callable>
struct Runnable {
  using function_ptr_type = typename Callable::function_ptr_type;

  static auto_handler_t const idx;

  static constexpr function_ptr_type*
  get_function();

  Runnable() = default;
};

template <typename ActiveFnT>
auto_handler_t const Runnable<ActiveFnT>::idx = register_active_fn<Runnable<ActiveFnT>>();

}} // end namespace runtime::auto_registry

#define register_active_function(F)                                     \
  do {                                                                  \
    runtime::auto_registry::Runnable<decltype(                          \
      runtime::auto_registry::FunctorAdapter<decltype(F),&F>()          \
    )>::idx;                                                            \
  } while (0);

#define get_handler_active_function_expand(TYPE_F, ADD_F)               \
  runtime::auto_registry::Runnable<                                     \
    decltype(                                                           \
      runtime::auto_registry::FunctorAdapter<TYPE_F, ADD_F>()           \
    )>::idx;                                                            \

#define get_handler_active_function(F)                                  \
  get_handler_active_function_expand(decltype(F), std::addressof(F))

#endif /*__RUNTIME_TRANSPORT_AUTO_REGISTRY__*/
