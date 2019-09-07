/*
//@HEADER
// *****************************************************************************
//
//                            term_epoch_advanced.cc
//                           DARMA Toolkit v. 1.0.0
//                       DARMA/vt => Virtual Transport
//
// Copyright 2019 National Technology & Engineering Solutions of Sandia, LLC
// (NTESS). Under the terms of Contract DE-NA0003525 with NTESS, the U.S.
// Government retains certain rights in this software.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// * Redistributions of source code must retain the above copyright notice,
//   this list of conditions and the following disclaimer.
//
// * Redistributions in binary form must reproduce the above copyright notice,
//   this list of conditions and the following disclaimer in the documentation
//   and/or other materials provided with the distribution.
//
// * Neither the name of the copyright holder nor the names of its
//   contributors may be used to endorse or promote products derived from this
//   software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
// ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
// LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
// CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
// SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
// INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
// CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
// ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
// POSSIBILITY OF SUCH DAMAGE.
//
// Questions? Contact darma@sandia.gov
//
// *****************************************************************************
//@HEADER
*/

#include "vt/transport.h"

#include <cstdlib>

using namespace vt;

static NodeType my_node = uninitialized_destination;
static NodeType num_nodes = uninitialized_destination;

using TTLType = int32_t;

static constexpr TTLType const max_life_time = 5;
static constexpr TTLType const no_ttl = -1000;

struct PropagateMsg : vt::Message {
  NodeType from = uninitialized_destination;
  TTLType life_time = no_ttl;

  explicit PropagateMsg(NodeType const& in_from)
    : ::vt::Message(), from(in_from), life_time(drand48() * max_life_time)
  { }

  PropagateMsg(NodeType const& in_from, TTLType const& in_ttl)
    : ::vt::Message(), from(in_from), life_time(in_ttl)
  { }
};

static void sendMsgEpoch(
  EpochType const& epoch, bool const set_epoch = false,
  TTLType const& ttl = no_ttl
);

static void propagateHandler(PropagateMsg* msg) {
  EpochType const epoch = envelopeGetEpoch(msg->env);
  fmt::print(
    "{}: propagate_handler: msg={}, epoch={}\n", my_node, print_ptr(msg), epoch
  );
  if (msg->life_time > 0) {
    sendMsgEpoch(epoch, false, msg->life_time);
  }
}

static void sendMsgEpoch(
  EpochType const& epoch, bool const set_epoch, TTLType const& ttl
) {
  NodeType random_node = drand48() * num_nodes;
  while (random_node == my_node) {
    random_node = drand48() * num_nodes;
  }
  vtAssertExpr(random_node != my_node);

  PropagateMsg* msg = nullptr;

  if (ttl == no_ttl) {
    // generate a random TLL for the new msg
    msg = makeSharedMessage<PropagateMsg>(my_node);
  } else {
    msg = makeSharedMessage<PropagateMsg>(my_node, ttl - 1);
  }

  if (epoch != no_epoch && set_epoch) {
    theMsg()->setEpochMessage(msg, epoch);
  }

  fmt::print(
    "{}: sending msg: epoch={}, ttl={}, random_node={}\n",
    my_node, epoch, msg->life_time, random_node
  );

  theMsg()->sendMsg<PropagateMsg, propagateHandler>(random_node, msg);

  fmt::print(
    "{}: sendMsgEpoch: epoch={}, node={}, ttl={}\n",
    my_node, epoch, random_node, ttl
  );
}

int main(int argc, char** argv) {
  CollectiveOps::initialize(argc, argv);

  my_node = theContext()->getNode();
  num_nodes = theContext()->getNumNodes();

  if (num_nodes == 1) {
    CollectiveOps::output("requires at least 2 nodes");
    CollectiveOps::finalize();
    return 0;
  }

  if (my_node == 1) {
    auto const& rooted_new_epoch = theTerm()->makeEpochRooted();
    ::fmt::print("{}: new epoch={}\n", my_node, rooted_new_epoch);
    sendMsgEpoch(rooted_new_epoch, true, 5);
    theTerm()->addAction(rooted_new_epoch, [=]{
      ::fmt::print("{}: epoch={}, action\n", my_node, rooted_new_epoch);
    });
    theTerm()->finishedEpoch(rooted_new_epoch);
  }

  while (!rt->isTerminated()) {
    runScheduler();
  }

  CollectiveOps::finalize();

  return 0;
}
