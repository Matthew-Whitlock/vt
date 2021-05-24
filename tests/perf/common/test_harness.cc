/*
//@HEADER
// *****************************************************************************
//
//                               test_harness.cc
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

#include "test_harness.h"

#include <vt/collective/collective_ops.h>
#include <vt/scheduler/scheduler.h>
#include <vt/utils/memory/memory_usage.h>

namespace vt { namespace tests { namespace perf { namespace common {

/*
* Helper function which generates CSV file with the tests resutls.
* Example output:
* name,min,max,mean,
*
*/
void OutputToFile() { }

void PerfTestHarness::SetUp(int argc, char** argv) {
  CollectiveOps::initialize(argc, argv);
  my_node_ = theContext()->getNode();
}

void PerfTestHarness::TearDown() {
  vt::theSched()->runSchedulerWhile([] { return !rt->isTerminated(); });
  CollectiveOps::finalize();
}

void PerfTestHarness::StartTimer() {
  watch_.Start();
}

void PerfTestHarness::StopTimer() {
  auto const time_elapsed = watch_.Stop();
  fmt::print("Node:{} {} took {}ms\n", my_node_, name_, time_elapsed.count());
}

void PerfTestHarness::GetMemoryUsage() {
  auto const mem_usage = vt::theMemUsage()->getUsageAll();
  fmt::print("Node:{} {} memory usage {}\n", my_node_, "name", mem_usage);
}

}}}} // namespace vt::tests::perf::common
