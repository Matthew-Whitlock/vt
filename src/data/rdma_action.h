
#if ! defined __RUNTIME_TRANSPORT_RDMA_ACTION__
#define __RUNTIME_TRANSPORT_RDMA_ACTION__

#include "common.h"
#include "rdma_common.h"

#include <sstream>
#include <iostream>

namespace runtime { namespace rdma {

struct Action {
  using action_count_t = int;

  action_count_t num_waiting = 0;
  action_t action_to_trigger = nullptr;

  Action(
    action_count_t const& num_waiting_in, action_t in_action_to_trigger
  ) : num_waiting(num_waiting_in), action_to_trigger(in_action_to_trigger)
  { }

  void add_dep() {
    num_waiting++;
  }

  void release() {
    num_waiting--;
    if (num_waiting == 0) {
      if (action_to_trigger) {
        action_to_trigger();
      }
      delete this;
    }
  }
};

}} //end namespace runtime::rdma

#endif /*__RUNTIME_TRANSPORT_RDMA_ACTION__*/
