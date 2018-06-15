
#if !defined INCLUDED_VRT_COLLECTION_HOLDERS_ENTIRE_HOLDER_H
#define INCLUDED_VRT_COLLECTION_HOLDERS_ENTIRE_HOLDER_H

#include "config.h"
#include "vrt/collection/holders/col_holder.h"

#include <unordered_map>
#include <unordered_set>
#include <memory>

namespace vt { namespace vrt { namespace collection {

template <typename=void>
struct UniversalIndexHolder {
  static void destroyAllLive() {
    for (auto&& elm : live_collections_) {
      elm->destroy();
    }
    live_collections_.clear();
  }

  static bool readyNextPhase() {
    return num_collections_phase_ == getNumCollections();
  }

  static void makeCollectionReady() {
    num_collections_phase_++;
  }

  static void resetPhase() {
    num_collections_phase_ = 0;
  }

  static std::size_t getNumCollections() {
    return live_collections_.size();
  }

public:
  static std::unordered_set<std::shared_ptr<BaseHolder>> live_collections_;
private:
  static std::size_t num_collections_phase_;
};

template <typename always_void_>
/*static*/ std::unordered_set<std::shared_ptr<BaseHolder>>
UniversalIndexHolder<always_void_>::live_collections_;

template <typename always_void_>
/*static*/ std::size_t
UniversalIndexHolder<always_void_>::num_collections_phase_ = 0;

template <typename ColT, typename IndexT>
struct EntireHolder {
  using InnerHolder = CollectionHolder<ColT, IndexT>;
  using InnerHolderPtr = std::shared_ptr<InnerHolder>;
  using ProxyContainerType = std::unordered_map<
    VirtualProxyType, InnerHolderPtr
  >;

  static void insert(VirtualProxyType const& proxy, InnerHolderPtr ptr) {
    proxy_container_.emplace(
      std::piecewise_construct,
      std::forward_as_tuple(proxy),
      std::forward_as_tuple(ptr)
    );
    UniversalIndexHolder<>::live_collections_.insert(ptr);
  }

  static ProxyContainerType proxy_container_;
};

template <typename ColT, typename IndexT>
/*static*/ typename EntireHolder<ColT, IndexT>::ProxyContainerType
EntireHolder<ColT, IndexT>::proxy_container_;

}}} /* end namespace vt::vrt::collection */

#endif /*INCLUDED_VRT_COLLECTION_HOLDERS_ENTIRE_HOLDER_H*/
