
#if ! defined __RUNTIME_TRANSPORT_RDMA_GROUP__
#define __RUNTIME_TRANSPORT_RDMA_GROUP__

#include "common.h"
#include "rdma_map.h"
#include "rdma_region.h"

#include <unordered_map>

namespace runtime { namespace rdma {

struct Group {
  using rdma_map_t = Map;
  using rdma_region_t = Region;

  Group(
    rdma_map_t const& in_map, rdma_elm_t const& in_total_elms,
    rdma_block_t const& in_num_blocks, byte_t const& in_elm_size
  ) : map(in_map), num_total_elems(in_total_elms),
      num_blocks(in_num_blocks), elm_size(in_elm_size)
  { }

  void
  set_map(rdma_map_t const& map);

  rdma_map_t
  get_map() const;

  template <typename Walker>
  void
  walk_region(rdma_region_t const& region, Walker&& walk) {
    rdma_elm_t lo = region.lo;
    rdma_elm_t hi = region.hi;
    rdma_elm_t elm_lo, elm_hi = lo;
    rdma_block_t blk_lo;
    node_t cur_node;

    assert(map.elm_map != nullptr);
    assert(map.block_map != nullptr);

    while (elm_hi < hi) {
      auto const& ret = map.elm_map(lo, num_total_elems, num_blocks);
      blk_lo = std::get<0>(ret);
      elm_lo = std::get<1>(ret);
      elm_hi = std::get<2>(ret);
      cur_node = map.block_map(blk_lo, num_blocks);

      walk(cur_node, ret, lo, std::min(elm_hi,hi));

      lo = elm_hi;
    }
  }

  rdma_map_t map;

  byte_t elm_size;
  rdma_elm_t num_total_elems = no_rdma_elm;
  rdma_block_t num_blocks = no_rdma_block;
};

}} //end namespace runtime::rdma

#endif /*__RUNTIME_TRANSPORT_RDMA_GROUP__*/
