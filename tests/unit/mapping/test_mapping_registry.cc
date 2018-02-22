
#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "test_parallel_harness.h"

#include "transport.h"
#include "topos/index/index.h"
#include "topos/mapping/mapping.h"
#include "registry/auto/auto_registry_map.h"

#include <vector>

namespace vt { namespace tests { namespace unit {

struct TestMsg : vt::ShortMessage {
  bool is_block = false;
  HandlerType han;
  TestMsg(HandlerType const& in_han) : ShortMessage(), han(in_han) { }
};

struct TestMappingRegistry : TestParallelHarness {
  static void test_handler(TestMsg* msg) {
    auto const& this_node = theContext()->getNode();
    //printf("%d: test_handler: han=%d\n", this_node, msg->han);
    auto fn = auto_registry::getAutoHandlerMap(msg->han);

    static constexpr index::Index1D::DenseIndexType const val = 64;
    static constexpr index::Index1D::DenseIndexType const max = 256;
    static constexpr NodeType const nnodes = 8;

    index::Index1D idx(val);
    index::Index1D max_idx(max);

    auto const& node = fn(
      reinterpret_cast<index::BaseIndex*>(&idx),
      reinterpret_cast<index::BaseIndex*>(&max_idx),
      nnodes
    );

    if (msg->is_block) {
      EXPECT_EQ(val / (max / nnodes), node);
    } else {
      EXPECT_EQ(val % nnodes, node);
    }
    //printf("node=%d\n", node);
  }

};

static NodeType map_fn(
  index::Index1D* idx, index::Index1D* max_idx, NodeType nnodes
) {
  return mapping::dense1DBlockMap(idx, max_idx, nnodes);
}

static NodeType map_fn2(
  index::Index1D* idx, index::Index1D* max_idx, NodeType nnodes
) {
  return mapping::dense1DRoundRobinMap(idx, max_idx, nnodes);
}

TEST_F(TestMappingRegistry, test_mapping_block_1d_registry) {
  auto const& my_node = theContext()->getNode();

  #if DEBUG_TEST_HARNESS_PRINT
    printf("test_type_safe_active_fn_send: node=%d\n", my_node);
  #endif

  if (my_node == 0) {
    auto map_han = auto_registry::makeAutoHandlerMap<index::Index1D, map_fn>();
    auto msg = new TestMsg(map_han);
    msg->is_block = true;
    theMsg()->broadcastMsg<TestMsg, test_handler>(msg, [=]{ delete msg; });

    auto map_han2 = auto_registry::makeAutoHandlerMap<index::Index1D, map_fn2>();
    auto msg2 = new TestMsg(map_han2);
    theMsg()->broadcastMsg<TestMsg, test_handler>(msg2, [=]{ delete msg2; });
  }
}

}}} // end namespace vt::tests::unit
