/*
//@HEADER
// *****************************************************************************
//
//                                 rotatelb.cc
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

#include "vt/config.h"
#include "vt/timing/timing.h"
#include "vt/vrt/collection/balance/rotatelb/rotatelb.h"
#include "vt/vrt/collection/manager.h"

#include <memory>

namespace vt { namespace vrt { namespace collection { namespace lb {

void RotateLB::init(objgroup::proxy::Proxy<RotateLB> in_proxy) {
  proxy = in_proxy;
}

/*static*/ std::unordered_map<std::string, std::string>
RotateLB::getInputKeysWithHelp() {
  return std::unordered_map<std::string, std::string>{};
}

void RotateLB::inputParams(balance::ConfigEntry* config) { }

void RotateLB::runLB(TimeType) {
  auto const& this_node = theContext()->getNode();
  auto const& num_nodes = theContext()->getNumNodes();
  auto const next_node = this_node + 1 > num_nodes-1 ? 0 : this_node + 1;

  if (this_node == 0) {
    vt_debug_print(
      terse, lb,
      "RotateLB: runLB: next_node={}\n",
      next_node
    );
    fflush(stdout);
  }

  for (auto obj : *load_model_) {
    TimeTypeWrapper const load = load_model_->getModeledLoad(
      obj, {balance::PhaseOffset::NEXT_PHASE, balance::PhaseOffset::WHOLE_PHASE}
    );
    vt_debug_print(
      terse, lb,
      "\t RotateLB::migrating object to: obj={}, load={}, to_node={}\n",
      obj, load, next_node
    );
    if (obj.isMigratable()) {
      migrateObjectTo(obj, next_node);
    }
  }
}

}}}} /* end namespace vt::vrt::collection::lb */

