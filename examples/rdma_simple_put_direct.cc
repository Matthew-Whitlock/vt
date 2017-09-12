
#include "transport.h"
#include <cstdlib>

using namespace vt;

static NodeType my_node = uninitialized_destination;
static NodeType num_nodes = uninitialized_destination;

static RDMA_HandleType my_handle_1 = no_rdma_handle;

static int const put_len = 2;
static int const my_data_len = 8;
static double* my_data = nullptr;

struct TestMsg : vt::Message {
  RDMA_HandleType han;
  TestMsg(RDMA_HandleType const& in_han) : Message(), han(in_han) { }
};

static void read_data_fn(TestMsg* msg) {
  printf("%d: read_data_fn: handle=%lld\n", my_node, msg->han);

  for (auto i = 0; i < put_len*2; i++) {
    printf("%d: han=%lld \t: my_data[%d] = %f\n", my_node, msg->han, i, my_data[i]);
  }
}

static void put_data_fn(TestMsg* msg) {
  if (my_node == 1 or my_node == 2) {
    printf(
      "%d: putting data, handle=%lld, my_data=%p\n",
      my_node, msg->han, my_data
    );

    int const num_elm = 2;
    int const offset = num_elm*(my_node-1);
    theRDMA->put_typed_data(msg->han, my_data, num_elm, offset, no_action, [=]{
      printf("%d: after put: sending msg back to 0: offset=%d\n", my_node, offset);

      TestMsg* back = make_shared_message<TestMsg>(msg->han);
      theMsg->send_msg<TestMsg, read_data_fn>(0, back);
    });
  }
}

int main(int argc, char** argv) {
  CollectiveOps::initialize_context(argc, argv);
  CollectiveOps::initialize_runtime();

  my_node = theContext->get_node();
  num_nodes = theContext->get_num_nodes();

  if (num_nodes < 4) {
    fprintf(stderr, "requires at least 4 nodes\n");
    return 0;
  }

  my_data = new double[my_data_len];

  // initialize my_data buffer, all but node 0 get -1.0
  for (auto i = 0; i < my_data_len; i++) {
    my_data[i] = my_node != 0 ? (my_node+1)*i+1 : -1.0;
  }

  if (my_node == 0) {
    my_handle_1 = theRDMA->register_new_typed_rdma_handler(my_data, put_len);

    printf(
      "%d: initializing my_handle_1=%lld\n",
      my_node, my_handle_1
    );

    TestMsg* msg1 = make_shared_message<TestMsg>(my_handle_1);
    TestMsg* msg2 = make_shared_message<TestMsg>(my_handle_1);
    theMsg->send_msg<TestMsg, put_data_fn>(1, msg1);
    theMsg->send_msg<TestMsg, put_data_fn>(2, msg2);
  }

  while (1) {
    run_scheduler();
  }

  return 0;
}
