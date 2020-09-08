/*
//@HEADER
// *****************************************************************************
//
//                          test_mpi_access_guards.cc
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

#include <gtest/gtest.h>

#include "vt/transport.h"
#include "test_parallel_harness.h"

namespace vt { namespace tests { namespace unit {

struct TestMpiAccessGuardTest : TestParallelHarness { };

using TestMpiAccessGuardDeathTest = TestMpiAccessGuardTest;

using DummyMsg = vt::Message;

static bool expected_to_fail_on_mpi_access = false;
static bool explicitly_grant_access = false;

static void attempt_mpi_access() {
  MPI_Request req = MPI_REQUEST_NULL;
  int flag;

  if (explicitly_grant_access) {
    VT_ALLOW_MPI_CALLS;
     MPI_Test(&req, &flag, MPI_STATUS_IGNORE);
  } else {
    // Should vtAssert if guard feature enabled as access not scope-granted.
    MPI_Test(&req, &flag, MPI_STATUS_IGNORE);
  }
}

static void message_handler(DummyMsg* msg) {
  if (expected_to_fail_on_mpi_access) {
    ASSERT_DEATH(
      attempt_mpi_access(),
      "MPI functions should not used inside user code invoked from VT handlers"
    );
  } else {
    attempt_mpi_access();
    SUCCEED();
  }
}

void testMpiAccess(bool access_allowed, bool grant_access) {
  expected_to_fail_on_mpi_access = not access_allowed;
  explicitly_grant_access = grant_access;

  vtAssert(theContext()->getNumNodes() >= 2, "Requires at least 2 nodes");

  if (theContext()->getNode() == 0) {
    // Sending message for common-case of attempting MPI access within
    // a message handler; it applies to all cases through the scheduler.
    auto msg = vt::makeMessage<DummyMsg>();
    theMsg()->sendMsg<DummyMsg, message_handler>(1, msg);
  }
}

TEST_F(TestMpiAccessGuardDeathTest, test_mpi_access_prevented) {
#if vt_check_enabled(mpi_access_guards) && !vt_check_enabled(production)
  testMpiAccess(false, false);
#else
  // Not applicable to 'death' / failure. Covered in allowed test case:
  // - must have feature enabled
  // - must be debug (as vtAssert is disabled in release/production)
  GTEST_SKIP();
#endif
}

TEST_F(TestMpiAccessGuardTest, test_mpi_access_allowed) {
#if vt_check_enabled(mpi_access_guards)
  // Only allowed with grant when feature enabled
  testMpiAccess(true, true);
#else
  // Always allowed when feature is not enabled
  testMpiAccess(true, true);
  testMpiAccess(true, false);
#endif
}

}}} /* end namespace vt::tests::unit */
