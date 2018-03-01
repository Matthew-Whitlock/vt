
#if !defined INCLUDED_VRT_COLLECTION_TYPES_BASE_IMPL_H
#define INCLUDED_VRT_COLLECTION_TYPES_BASE_IMPL_H

#include "config.h"
#include "vrt/vrt_common.h"
#include "vrt/collection/types/base.h"
#include "vrt/collection/migrate/manager_migrate_attorney.h"
#include "vrt/collection/migrate/migrate_status.h"

#include <cassert>

namespace vt { namespace vrt { namespace collection {

template <typename IndexT>
CollectionBase<IndexT>::CollectionBase(
  bool const static_size, bool const elms_fixed
) : Indexable<IndexT>(),
    hasStaticSize_(static_size),
    elmsFixedAtCreation_(elms_fixed)
{ }

template <typename IndexT>
typename CollectionBase<IndexT>::ProxyType
CollectionBase<IndexT>::getElementProxy(IndexT const& idx) const {
  VirtualElmOnlyProxyType elmProxy;
  VirtualElemProxyBuilder::createElmProxy(elmProxy, idx.uniqueBits());
  ProxyType proxy(this->getProxy(), elmProxy);
  return proxy;
}

template <typename IndexT>
bool CollectionBase<IndexT>::isStatic() const {
  return hasStaticSize_ && elmsFixedAtCreation_;
}

template <typename IndexT>
/*static*/ bool CollectionBase<IndexT>::isStaticSized() {
  return true;
}

template <typename IndexT>
template <typename ColT>
void CollectionBase<IndexT>::migrate(NodeType const& node) {
  auto const& collection_proxy = this->getProxy();
  auto const& collection_index = this->getIndex();
  auto const& migrate_status = CollectionElmAttorney<ColT,IndexT>::migrateOut(
    collection_proxy, collection_index, node
  );
  assert(
    migrate_status == MigrateStatus::MigratedToRemote &&
    "Required be immediate, valid migration currently"
  );
}

template <typename IndexT>
template <typename Serializer>
void CollectionBase<IndexT>::serialize(Serializer& s) {
  Indexable<IndexT>::serialize(s);
  s | hasStaticSize_;
  s | elmsFixedAtCreation_;
}

template <typename IndexT>
/*virtual*/ CollectionBase<IndexT>::~CollectionBase() {}

}}} /* end namespace vt::vrt::collection */

#endif /*INCLUDED_VRT_COLLECTION_TYPES_BASE_IMPL_H*/
