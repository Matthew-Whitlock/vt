
#include "auto_registry_common.h"
#include "auto_registry_interface.h"
#include "config.h"
#include "registry.h"

namespace vt { namespace auto_registry {

#if backend_check_enabled(trace_enabled)
trace::TraceEntryIDType theTraceID(HandlerType const& handler) {
  auto const& han_id = HandlerManagerType::getHandlerIdentifier(handler);

  bool const& is_functor = HandlerManagerType::isHandlerFunctor(handler);

  if (not is_functor) {
    return theAutoRegistry().at(han_id).getTraceID();
  } else {
    return theAutoRegistryFunctor().at(han_id).getTraceID();
  }
}
#endif

}} // end namespace vt::auto_registry
