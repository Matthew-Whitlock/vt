
#if !defined INCLUDED_VRT_COLLECTION_MIGRATE_MIGRATE_HANDLERS_IMPL_H
#define INCLUDED_VRT_COLLECTION_MIGRATE_MIGRATE_HANDLERS_IMPL_H

#include "config.h"
#include "vrt/vrt_common.h"
#include "vrt/collection/migrate/migrate_msg.h"
#include "vrt/collection/migrate/migrate_handlers.h"
#include "vrt/collection/migrate/manager_migrate_attorney.h"
#include "serialization/serialization.h"

#include <memory>
#include <functional>
#include <cassert>

namespace vt { namespace vrt { namespace collection {

template <typename ColT, typename IndexT>
/*static*/ void MigrateHandlers::migrateInHandler(
  MigrateMsg<ColT, IndexT>* msg
) {
  using ColBaseType = typename Holder<IndexT>::CollectionType;
  using ColDeleterType = std::function<void(ColBaseType*)>;

  auto const& full_proxy = msg->getElementProxy();
  auto const& col_proxy = full_proxy.getCollectionProxy();
  auto const& elm_proxy = full_proxy.getElementProxy();
  auto const& from_node = msg->getFromNode();
  auto const& idx = elm_proxy.getIndex();
  auto const& map_han = msg->getMapHandler();
  auto const& range = msg->getRange();

  auto buf = reinterpret_cast<SerialByteType*>(msg->getPut());
  auto const& buf_size = msg->getPutSize();

  auto vc_elm_ptr = std::make_unique<ColT>();
  auto vc_raw_ptr = ::serialization::interface::deserialize<ColT>(
    buf, buf_size, vc_elm_ptr.get()
  );
  // ColT* col_t = new ColT();
  // auto vc_raw_ptr = ::serialization::interface::deserialize<ColT>(
  //   buf, buf_size, col_t
  // );
  // auto vc_elm_ptr = std::unique_ptr<ColT>(vc_raw_ptr);

  auto const& migrate_status =
    CollectionElmAttorney<ColT,IndexT>::migrateIn(
      col_proxy, idx, from_node, std::move(vc_elm_ptr), range, map_han
    );

  assert(
    migrate_status == MigrateStatus::MigrateInLocal &&
    "Should be valid local migration into this memory domain"
  );
}

}}} /* end namespace vt::vrt::collection */


#endif /*INCLUDED_VRT_COLLECTION_MIGRATE_MIGRATE_HANDLERS_IMPL_H*/
