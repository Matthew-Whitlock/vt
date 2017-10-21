
#if !defined INCLUDED_CONTEXT_VRT_MANAGER
#define INCLUDED_CONTEXT_VRT_MANAGER

#include <unordered_map>
#include <memory>

#include "config.h"
#include "context/context.h"
#include "context_vrt.h"
#include "context_vrt_funcs.h"
#include "context_vrtmessage.h"
#include "context_vrtinfo.h"
#include "context_vrtproxy.h"
#include "context_vrt_internal_msgs.h"

#include "utils/bits/bits_common.h"
#include "activefn/activefn.h"
#include "topos/mapping/mapping_function.h"
#include "serialization/serialization.h"

namespace vt { namespace vrt {

using namespace ::vt::serialization;

struct PendingRequest {
  VirtualRequestIDType req_id = no_request_id;

  ActionProxyType action = nullptr;

  PendingRequest(
    VirtualRequestIDType const& in_req_id, ActionProxyType in_action
  ) : req_id(in_req_id), action(in_action)
  { }
};

struct VirtualContextManager {
  using VirtualPtrType = std::unique_ptr<VirtualContext>;
  using PendingRequestType = PendingRequest;
  using VirtualInfoType = VirtualInfo;
  using ContainerType = std::unordered_map<VirtualIDType, VirtualInfoType>;
  using ContainerRemoteType = std::unordered_map<VirtualIDType, VirtualInfoType>;
  using PendingContainerType = std::unordered_map<VirtualRequestIDType, PendingRequest>;

  VirtualContextManager();

  template <typename VrtContextT, typename... Args>
  VirtualProxyType makeVirtual(Args&& ... args);

  template <typename VrtContextT, typename... Args>
  VirtualProxyType makeVirtualNode(NodeType const& node, Args&& ... args);

  template <typename VrtContextT, mapping::ActiveSeedMapFnType fn, typename... Args>
  VirtualProxyType makeVirtualMap(Args&& ... args);

  VirtualContext* getVirtualByProxy(VirtualProxyType const& proxy);
  void destoryVirtualByProxy(VirtualProxyType const& proxy);

  template <typename VcT, typename MsgT, ActiveVrtTypedFnType<MsgT, VcT> *f>
  void sendMsg(
    VirtualProxyType const& toProxy, MsgT *const msg, ActionType act = nullptr
  );

  template <typename VcT, typename MsgT, ActiveVrtTypedFnType<MsgT, VcT> *f>
  void sendSerialMsg(
    VirtualProxyType const& toProxy, MsgT *const msg, ActionType act = nullptr
  );

private:
  template <typename VrtContextT, typename... Args>
  VirtualProxyType makeVirtualRemote(
    NodeType const& node, bool isImmediate, ActionProxyType action,
    Args&&... args
  );

  void insertVirtualContext(VirtualPtrType new_vc, VirtualProxyType proxy);

  VirtualProxyType generateNewProxy();
  VirtualRemoteIDType generateNewRemoteID(NodeType const& node);

  // All messages directed to a virtual context are routed through this handler
  // so the user's handler can be invoked with the pointer to the virtual
  // context
  static void virtualMsgHandler(BaseMessage* msg);

  template <typename MsgT>
  static void virtualTypedMsgHandler(MsgT* msg);

  // Handler to send back a generated proxy to a requesting node
  static void sendBackVirtualProxyHan(VirtualProxyRequestMsg* msg);

  template <typename VrtCtxT, typename Tuple, size_t... I>
  static VirtualPtrType runConstructor(Tuple* tup, std::index_sequence<I...>);

  template <typename SysMsgT>
  static void remoteConstructVrt(SysMsgT* msg);

  void recvVirtualProxy(VirtualRequestIDType id, VirtualProxyType proxy);

  VirtualContext* getVirtualByID(VirtualIDType const& lookupID, bool const is_remote);
  void destroyVirtualByID(VirtualIDType const& lookupID, bool const is_remote);
  VirtualIDType getCurrentID() const;
  NodeType getNode() const;

 private:
  // Holder for local virtual contexts that are mapped to this node; VirtualInfo
  // holds a pointer to the virtual along with other meta-information about it
  ContainerType holder_;
  ContainerRemoteType remote_holder_;

  SeedType cur_seed_ = 0;

  // The current identifier (node-local) for this manager
  VirtualIDType curIdent_;

  // The current identifier (for remote nodes) for this manager
  std::unordered_map<NodeType, VirtualRemoteIDType> curRemoteID_;

  // Cache of the node for the virtual context manager
  NodeType myNode_;

  // The current request identifier, used to track remote transactions
  VirtualRequestIDType cur_request_id = 0;

  PendingContainerType pending_request_;
};

}}  // end namespace vt::vrt

namespace vt {

extern vrt::VirtualContextManager* theVirtualManager();

}  // end namespace vt

#include "context_vrtmanager.impl.h"

#endif  /*INCLUDED_CONTEXT_VRT_MANAGER*/
