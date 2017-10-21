
#if !defined INCLUDED_REGISTRY_AUTO_REGISTRY_COMMON_H
#define INCLUDED_REGISTRY_AUTO_REGISTRY_COMMON_H

#include "trace/trace_event.h"

#include "config.h"
#include "activefn/activefn.h"
#include "registry.h"
#include "trace/trace.h"
#include "vrt/context/context_vrt_funcs.h"
#include "topos/mapping/mapping_function.h"

#include <vector>

namespace vt { namespace auto_registry {

using AutoActiveType = ActiveFnPtrType;
using AutoActiveFunctorType = ActiveFnPtrType;
using AutoActiveVCType = vrt::ActiveVirtualFnPtrType;
using AutoActiveMapType = mapping::ActiveMapFnPtrType;
using AutoActiveSeedMapType = mapping::ActiveSeedMapFnPtrType;

using HandlerManagerType = vt::HandlerManager;
using AutoHandlerType = int32_t;

template <typename FnT>
struct AutoRegInfo {
  FnT activeFunT;

  #if backend_check_enabled(trace_enabled)

  trace::TraceEntryIDType event_id;

  AutoRegInfo(
    FnT const& in_active_fun_t, trace::TraceEntryIDType const& in_event_id
  ) : activeFunT(in_active_fun_t), event_id(in_event_id)
  { }

  trace::TraceEntryIDType theTraceID() const {
    return event_id;
  }

  #else

  AutoRegInfo(FnT const& in_active_fun_t)
    : activeFunT(in_active_fun_t) { }

  #endif

  FnT getFun() const {
    return activeFunT;
  }
};

template <typename Fn>
using AutoRegInfoType = AutoRegInfo<Fn>;

template <typename RegInfoT>
using AutoRegistryContainerType = std::vector<AutoRegInfoType<RegInfoT>>;

using AutoActiveContainerType = AutoRegistryContainerType<AutoActiveType>;
using AutoActiveVCContainerType = AutoRegistryContainerType<AutoActiveVCType>;
using AutoActiveMapContainerType = AutoRegistryContainerType<AutoActiveMapType>;
using AutoActiveSeedMapContainerType = AutoRegistryContainerType<AutoActiveSeedMapType>;
using AutoActiveFunctorContainerType = AutoRegistryContainerType<AutoActiveFunctorType>;

}} // end namespace vt::auto_registry

#endif /*INCLUDED_REGISTRY_AUTO_REGISTRY_COMMON_H*/
