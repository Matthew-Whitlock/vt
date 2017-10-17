
#if !defined INCLUDED_CONTEXT_VRT_MANAGER_IMPL
#define INCLUDED_CONTEXT_VRT_MANAGER_IMPL

#include "config.h"
#include "context_vrtmanager.h"
#include "context_vrt_internal_msgs.h"
#include "context_vrt_remoteinfo.h"
#include "topos/location/location.h"
#include "registry/auto_registry_vc.h"
#include "registry/auto_registry_map.h"
#include "serialization/serialization.h"

#include <cassert>
#include <memory>

namespace vt { namespace vrt {

template <typename VrtContextT, typename... Args>
VirtualProxyType VirtualContextManager::makeVirtual(Args&& ... args) {
  auto new_vc = std::make_unique<VrtContextT>(std::forward<Args>(args)...);
  auto const& proxy = generateNewProxy();
  insertVirtualContext(std::move(new_vc), proxy);
  return proxy;
}

using namespace ::vt::serialization;

template <typename VrtCtxT, typename Tuple, size_t... I>
/*static*/ typename VirtualContextManager::VirtualPtrType
VirtualContextManager::runConstructor(
  Tuple* tup, std::index_sequence<I...>
) {
  return std::make_unique<VrtCtxT>(
    std::forward<typename std::tuple_element<I,Tuple>::type>(
      std::get<I>(*tup)
    )...
  );
}

template <typename SysMsgT>
/*static*/ void VirtualContextManager::remoteConstructVrt(SysMsgT* msg) {
  using VrtContextT = typename SysMsgT::VirtualContextType;
  using Args = typename SysMsgT::ArgsTupleType;

  static constexpr auto size = std::tuple_size<Args>::value;
  auto new_vc = VirtualContextManager::runConstructor<VrtContextT>(
    &msg->tup, std::make_index_sequence<size>{}
  );

  auto const& info = msg->info;
  VirtualProxyType new_proxy = info.proxy;

  if (info.isImmediate) {
    // nothing to do here?
  } else {
    auto const& cons_node = theContext->getNode();
    auto const& req_node = info.from_node;
    auto const& request_id = info.req_id;

    new_proxy = theVirtualManager->generateNewProxy();
    auto send_msg = makeSharedMessage<VirtualProxyRequestMsg>(
      cons_node, req_node, request_id, new_proxy
    );
    theMsg->sendMsg<VirtualProxyRequestMsg, sendBackVirtualProxyHan>(
      req_node, send_msg
    );
  }

  theVirtualManager->insertVirtualContext(std::move(new_vc), new_proxy);
}

template <typename VrtContextT, typename... Args>
VirtualProxyType VirtualContextManager::makeVirtualNode(
  NodeType const& node, Args&& ... args
) {
  auto const& this_node = theContext->getNode();
  if (node != this_node) {
    return makeVirtualRemote<VrtContextT>(
      node, true, nullptr, std::forward<Args>(args)...
    );
  } else {
    return makeVirtual<VrtContextT>(std::forward<Args>(args)...);
  }
}

template <typename MsgT>
/*static*/ void VirtualContextManager::virtualTypedMsgHandler(MsgT* msg) {
  VirtualContextManager::virtualMsgHandler(reinterpret_cast<BaseMessage*>(msg));
}

template <typename VcT, typename MsgT, ActiveVrtTypedFnType<MsgT, VcT> *f>
void VirtualContextManager::sendSerialMsg(
  VirtualProxyType const& toProxy, MsgT *const msg, ActionType act
) {
  NodeType const& home_node = VirtualProxyBuilder::getVirtualNode(toProxy);
  // register the user's handler
  HandlerType const& han = auto_registry::makeAutoHandlerVC<VcT,MsgT,f>(msg);
  // save the user's handler in the message
  msg->setVrtHandler(han);
  msg->to_proxy = toProxy;

  debug_print(
    vrt, node,
    "sending serialized msg to VC: msg=%p, han=%d, home_node=%d, toProxy=%lld\n",
    msg, han, home_node, toProxy
  );

  using SerialMsgT = SerializedEagerMsg<MsgT, VirtualMessage>;

  // route the message to the destination using the location manager
  SerializedMessenger::sendSerialMsg<
    MsgT, virtualTypedMsgHandler<MsgT>, VirtualMessage
  >(
    msg,
    // custom send lambda to route the message
    [=](SerialMsgT* msg){
      msg->to_proxy = toProxy;
      theLocMan->vrtContextLoc->routeMsgHandler<
        SerialMsgT, SerializedMessenger::payloadMsgHandler
      >(toProxy, home_node, msg, act);
    },
    // custom data transfer lambda if above the eager threshold
    [=](ActionNodeType action){
      theLocMan->vrtContextLoc->routeNonEagerAction(toProxy, home_node, action);
    }
  );
}

template <typename VrtContextT, typename... Args>
VirtualProxyType VirtualContextManager::makeVirtualRemote(
  NodeType const& dest, bool isImmediate, ActionProxyType action, Args&&... args
) {
  using ArgsTupleType = std::tuple<typename std::decay<Args>::type...>;
  using MsgType = VrtConstructMsg<RemoteVrtInfo, ArgsTupleType, VrtContextT>;

  auto sys_msg =
    makeSharedMessage<MsgType>(ArgsTupleType{std::forward<Args>(args)...});

  auto const& this_node = theContext->getNode();
  std::unique_ptr<RemoteVrtInfo> info = nullptr;
  VirtualProxyType return_proxy = no_vrt_proxy;

  if (isImmediate) {
    auto const& remote_id = generateNewRemoteID(dest);
    return_proxy = VirtualProxyBuilder::createRemoteProxy(
      remote_id, myNode_, dest, false, false
    );
    info = std::make_unique<RemoteVrtInfo>(this_node, return_proxy);
  } else {
    auto next_req = cur_request_id++;

    // insert a pending request to trigger the action
    pending_request_.emplace(
      std::piecewise_construct,
      std::forward_as_tuple(next_req),
      std::forward_as_tuple(PendingRequestType{next_req, action})
    );

    info = std::make_unique<RemoteVrtInfo>(this_node, next_req);
  }

  sys_msg->info = *info.get();

  SerializedMessenger::sendSerialMsg<MsgType, remoteConstructVrt<MsgType>>(
    dest, sys_msg
  );

  return return_proxy;
}

template <typename VrtContextT, mapping::ActiveSeedMapFnType fn, typename... Args>
VirtualProxyType VirtualContextManager::makeVirtualMap(
  Args&& ... args
) {
  auto const& proxy = makeVirtual<VrtContextT, Args...>(
    std::forward<Args>(args)...
  );

  if (theContext->hasWorkers()) {
    // save the seed for mapping
    auto const next_seed = cur_seed_++;
    auto vrt = getVirtualByProxy(proxy);
    vrt->seed_ = next_seed;

    auto const& core_map_handle = auto_registry::makeAutoHandlerSeedMap<fn>();

    auto const& vrt_id = VirtualProxyBuilder::getVirtualID(proxy);
    auto holder_iter = holder_.find(vrt_id);
    assert(holder_iter != holder_.end() && "Proxy ID Must exist here");

    auto& info = holder_iter->second;
    info.setCoreMap(core_map_handle);

    auto const& mapped_core = fn(vrt->seed_, theContext->getNumWorkers());
    info.mapToCore(mapped_core);

    debug_print(
      vrt, node,
      "seed=%lld, mapped_core=%d\n", next_seed, mapped_core
    );
  }

  return proxy;
}

template <typename VcT, typename MsgT, ActiveVrtTypedFnType<MsgT, VcT> *f>
void VirtualContextManager::sendMsg(
  VirtualProxyType const& toProxy, MsgT *const msg, ActionType act
) {
  // @todo: implement the action `act' after the routing is finished

  NodeType const& home_node = VirtualProxyBuilder::getVirtualNode(toProxy);
  // register the user's handler
  HandlerType const& han = auto_registry::makeAutoHandlerVC<VcT,MsgT,f>(msg);
  // save the user's handler in the message
  msg->setVrtHandler(han);
  msg->to_proxy = toProxy;

  debug_print(
    vrt, node,
    "sending msg to VC: msg=%p, han=%d, home_node=%d\n", msg, han, home_node
  );

  // route the message to the destination using the location manager
  theLocMan->vrtContextLoc->routeMsg(toProxy, home_node, msg, act);
}

}}  // end namespace vt::vrt

#endif /*INCLUDED_CONTEXT_VRT_MANAGER_IMPL*/
