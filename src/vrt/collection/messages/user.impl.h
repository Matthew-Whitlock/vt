
#if !defined INCLUDED_VRT_COLLECTION_MESSAGES_USER_IMPL_H
#define INCLUDED_VRT_COLLECTION_MESSAGES_USER_IMPL_H

#include "config.h"
#include "vrt/collection/messages/user.h"

#include <cassert>

namespace vt { namespace vrt { namespace collection {

template <typename ColT, typename BaseMsgT>
void CollectionMessage<ColT, BaseMsgT>::setVrtHandler(
  HandlerType const& in_handler
) {
  vt_sub_handler_ = in_handler;
}

template <typename ColT, typename BaseMsgT>
HandlerType CollectionMessage<ColT, BaseMsgT>::getVrtHandler() const {
  assert(
    vt_sub_handler_ != uninitialized_handler && "Must have a valid handler"
  );
  return vt_sub_handler_;
}

template <typename ColT, typename BaseMsgT>
VirtualElmProxyType<ColT, typename ColT::IndexType>
CollectionMessage<ColT, BaseMsgT>::getProxy() const {
  return to_proxy_;
}

template <typename ColT, typename BaseMsgT>
void CollectionMessage<ColT, BaseMsgT>::setProxy(
  VirtualElmProxyType<ColT, typename ColT::IndexType> const& in_proxy
) {
  to_proxy_ = in_proxy;
}

template <typename ColT, typename BaseMsgT>
VirtualProxyType CollectionMessage<ColT, BaseMsgT>::getBcastProxy() const {
  return bcast_proxy_;
}

template <typename ColT, typename BaseMsgT>
void CollectionMessage<ColT, BaseMsgT>::setBcastProxy(
  VirtualProxyType const& in_proxy
) {
  bcast_proxy_ = in_proxy;
}

template <typename ColT, typename BaseMsgT>
EpochType CollectionMessage<ColT, BaseMsgT>::getBcastEpoch() const {
  return bcast_epoch_;
}

template <typename ColT, typename BaseMsgT>
void CollectionMessage<ColT, BaseMsgT>::setBcastEpoch(EpochType const& epoch) {
  bcast_epoch_ = epoch;
}

template <typename ColT, typename BaseMsgT>
template <typename SerializerT>
void CollectionMessage<ColT, BaseMsgT>::serialize(SerializerT& s) {
  RoutedMessageType<BaseMsgT, ColT>::serialize(s);
  s | vt_sub_handler_;
  s | to_proxy_;
}

}}} /* end namespace vt::vrt::collection */

#endif /*INCLUDED_VRT_COLLECTION_MESSAGES_USER_IMPL_H*/
