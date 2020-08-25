/*
//@HEADER
// *****************************************************************************
//
//                     test_model_linear_model.extended
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

#include <vt/transport.h>
#include <vt/vrt/collection/balance/model/load_model.h>
#include <vt/vrt/collection/balance/model/linear_model.h>

#include <gtest/gtest.h>

#include "test_parallel_harness.h"

#include <memory>

namespace vt { namespace tests { namespace unit {

using TestLinearModel = TestHarness;

static constexpr int32_t const num_phases = 4;

using vt::vrt::collection::balance::ElementIDType;
using vt::vrt::collection::balance::LoadModel;
using vt::vrt::collection::balance::LinearModel;
using vt::vrt::collection::balance::PhaseOffset;
using vt::vrt::collection::balance::LoadMapType;
using vt::vrt::collection::balance::SubphaseLoadMapType;
using vt::vrt::collection::balance::CommMapType;
using vt::vrt::collection::balance::ObjectIterator;

struct StubModel : LoadModel {

  StubModel() = default;
  virtual ~StubModel() = default;

  void setLoads(
    std::vector<LoadMapType> const* proc_load,
    std::vector<SubphaseLoadMapType> const*,
    std::vector<CommMapType> const*) override {
    proc_load_ = proc_load;
  }

  void updateLoads(PhaseType) override {}

  TimeType getWork(ElementIDType id, PhaseOffset phase) override {
    const auto index = -1 * phase.phases - 1;
    return proc_load_->at(index).at(id);
  }

  virtual ObjectIterator begin() override {
    return ObjectIterator(proc_load_->back().begin());
  }
  virtual ObjectIterator end() override {
    return ObjectIterator(proc_load_->back().end());
  }

  virtual int getNumObjects() override { return 2; }
  virtual int getNumCompletedPhases() override { return num_phases; }
  virtual int getNumSubphases() override { return 1; }

private:
  std::vector<LoadMapType> const* proc_load_ = nullptr;
};

TEST_F(TestLinearModel, test_model_linear_model_1) {
  std::vector<LoadMapType> proc_loads = {
    LoadMapType{
      {ElementIDType{1}, TimeType{10}}, {ElementIDType{2}, TimeType{40}}},
    LoadMapType{
      {ElementIDType{1}, TimeType{20}}, {ElementIDType{2}, TimeType{30}}},
    LoadMapType{
      {ElementIDType{1}, TimeType{30}}, {ElementIDType{2}, TimeType{20}}},
    LoadMapType{
      {ElementIDType{1}, TimeType{40}}, {ElementIDType{2}, TimeType{10}}}};

  auto test_model =
    std::make_shared<LinearModel>(std::make_shared<StubModel>(), num_phases);

  test_model->setLoads(&proc_loads, nullptr, nullptr);

  for (auto&& obj : *test_model) {
    auto work_val = test_model->getWork(obj, PhaseOffset{});
    EXPECT_EQ(work_val, obj == 1 ? TimeType{0} : TimeType{50});
  }
}

}}} // end namespace vt::tests::unit
