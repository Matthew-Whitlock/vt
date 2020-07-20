/*
//@HEADER
// *****************************************************************************
//
//                           select_subphases.cc
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

#include "vt/vrt/collection/balance/model/select_subphases.h"

namespace vt { namespace vrt { namespace collection { namespace balance {

SelectSubphases::SelectSubphases(std::shared_ptr<LoadModel> base, std::vector<unsigned int> subphases)
  : ComposedModel(base)
  , subphases_(subphases)
{
  unsigned int base_subphases = ComposedModel::getNumSubphases();
  for (auto s : subphases) {
    vtAssert(s != PhaseOffset::WHOLE_PHASE, "Selecting WHOLE_PHASE as a subphase makes no sense");
    vtAssert(s < base_subphases, "Selected subphase must be within range of base's known subphases");
  }
  // Don't check that the 'select' subphases are a smaller set than
  // the base - allow multiple counting or other potential cleverness
  //vtAssert(subphases_.size() < base_subphases, "...");
}

TimeType SelectSubphases::getWork(ElementIDType object, PhaseOffset when) {
  if (when.subphase == PhaseOffset::WHOLE_PHASE) {
    // sum up the selected subphases
    TimeType sum = 0.0;
    for (auto s : subphases_) {
      PhaseOffset p{when.phases, s};
      sum += ComposedModel::getWork(object, p);
    }
    return sum;
  } else {
    when.subphase = subphases_.at(when.subphase);
    return ComposedModel::getWork(object, when);
  }
}

int SelectSubphases::getNumSubphases() {
  return subphases_.size();
}

}}}}
