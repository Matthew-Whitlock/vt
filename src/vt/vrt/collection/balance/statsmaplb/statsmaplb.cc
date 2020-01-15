/*
//@HEADER
// ************************************************************************
//
//                          StatsMapLB.cc
//                     vt (Virtual Transport)
//                  Copyright (C) 2018 NTESS, LLC
//
// Under the terms of Contract DE-NA-0003525 with NTESS, LLC,
// the U.S. Government retains certain rights in this software.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are
// met:
//
// 1. Redistributions of source code must retain the above copyright
// notice, this list of conditions and the following disclaimer.
//
// 2. Redistributions in binary form must reproduce the above copyright
// notice, this list of conditions and the following disclaimer in the
// documentation and/or other materials provided with the distribution.
//
// 3. Neither the name of the Corporation nor the names of the
// contributors may be used to endorse or promote products derived from
// this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY SANDIA CORPORATION "AS IS" AND ANY
// EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
// PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL SANDIA CORPORATION OR THE
// CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
// EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
// PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
// PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
// LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
// NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
// SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
// Questions? Contact darma@sandia.gov
//
// ************************************************************************
//@HEADER
*/

#include "vt/config.h"
#include "vt/vrt/collection/balance/baselb/baselb.h"
#include "vt/vrt/collection/balance/proc_stats.h"
#include "vt/vrt/collection/balance/statsmaplb/statsmaplb.h"
#include "vt/context/context.h"

namespace vt { namespace vrt { namespace collection { namespace lb {

void StatsMapLB::init(objgroup::proxy::Proxy<StatsMapLB> in_proxy) {
  proxy_ = in_proxy;
}

void StatsMapLB::runLB() {

  auto epoch = startMigrationCollective();
  theMsg()->pushEpoch(epoch);

  auto &myNewList = balance::ProcStats::proc_move_list_[phase_];
  for (size_t in = 0; in < myNewList.size(); in += 2) {
    auto iter = balance::ProcStats::proc_perm_to_temp_.find(myNewList[in]);
    if (iter != balance::ProcStats::proc_perm_to_temp_.end()) {
      migrateObjectTo(iter->second, myNewList[in+1]);
    }
    else {
      vtAssertExpr(iter != balance::ProcStats::proc_perm_to_temp_.end());
    }
  }

  theMsg()->popEpoch(epoch);
  finishMigrationCollective();

  myNewList.clear();

}

}}}} /* end namespace vt::vrt::collection::lb */
