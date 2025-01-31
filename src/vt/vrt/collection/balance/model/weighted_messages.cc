/*
//@HEADER
// *****************************************************************************
//
//                             weighted_messages.cc
//                       DARMA/vt => Virtual Transport
//
// Copyright 2019-2021 National Technology & Engineering Solutions of Sandia, LLC
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

#include "vt/vrt/collection/balance/model/weighted_messages.h"

namespace vt { namespace vrt { namespace collection { namespace balance {

TimeType
WeightedMessages::getModeledComm(ElementIDStruct object, PhaseOffset when) const {
  auto phase = getNumCompletedPhases() + when.phases;
  auto& comm = proc_comm_->at(phase);

  TimeType incoming = 0., outgoing = 0.;
  for (auto&& c : comm) {
    if (
      c.first.commCategory() == elm::CommCategory::SendRecv and
      c.first.offNode()) {
      if (c.first.toObj() == object) {
        incoming += per_msg_weight_ * c.second.messages;
        incoming += per_byte_weight_ * c.second.bytes;
      } else if (c.first.fromObj() == object) {
        outgoing += per_msg_weight_ * c.second.messages;
        outgoing += per_byte_weight_ * c.second.bytes;
      }
    }
  }

  auto modeled_comm =
    ComposedModel::getModeledComm(object, when) + std::max(incoming, outgoing);

  if (when.subphase == PhaseOffset::WHOLE_PHASE) {
    return modeled_comm;
  } else {
    // we don't record comm costs for each subphase - split it proportionally
    auto load = ComposedModel::getModeledLoad(object, when);
    auto whole_phase_load = ComposedModel::getModeledLoad(
      object, PhaseOffset{when.phases, PhaseOffset::WHOLE_PHASE});

    return modeled_comm * load / whole_phase_load;
  }
}

}}}} // namespace vt::vrt::collection::balance
