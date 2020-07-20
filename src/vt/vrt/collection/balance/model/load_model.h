/*
//@HEADER
// *****************************************************************************
//
//                                 load_model.h
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

#if !defined INCLUDED_VRT_COLLECTION_BALANCE_LOAD_MODEL_H
#define INCLUDED_VRT_COLLECTION_BALANCE_LOAD_MODEL_H

#include "vt/config.h"
#include "vt/vrt/collection/balance/lb_common.h"
#include "vt/vrt/collection/balance/lb_comm.h"

namespace vt { namespace vrt { namespace collection { namespace balance {

/**
 * \brief A description of the interval of interest for a modeled load query
 *
 * The value of `phases` can be in the past or future. Negative values
 * represent a distance into the past, in which -1 is most recent. A
 * value of 0 represents the immediate upcoming phase. Positive values
 * represent more distant future phases.
 */
struct PhaseOffset {
  int phases;
  static constexpr unsigned int NEXT_PHASE = 0;

  unsigned int subphase;
  static constexpr unsigned int WHOLE_PHASE = ~0u;
};

class ObjectIterator {
  using difference_type = std::ptrdiff_t;
  using value_type = LoadMapType::key_type;
  using pointer = value_type*;
  using reference = value_type&;

  using map_iterator_type = LoadMapType::const_iterator;
  using iterator_category = std::iterator_traits<map_iterator_type>::iterator_category;
  map_iterator_type i;

public:
  ObjectIterator(map_iterator_type in) : i(in) { }
  void operator++() { ++i; }
  value_type operator*() { return i->first; }
  bool operator!=(ObjectIterator rhs) { return i != rhs.i; }
  difference_type operator-(ObjectIterator rhs) { return std::distance(rhs.i, i); }
};

class LoadModel
{
public:
  LoadModel() {}

  /**
   * \internal \brief Initialize the model instance with pointers to the measured load data
   *
   * This would typically be called by LBManager when the user has
   * passed a new model instance for a collection
   */
  void setLoads(std::vector<LoadMapType> const* proc_load,
		std::vector<SubphaseLoadMapType> const* proc_subphase_load,
		std::vector<CommMapType> const* proc_comm)
  {
    proc_load_ = proc_load;
    proc_subphase_load_ = proc_subphase_load;
    proc_comm_ = proc_comm;
  }

  /**
   * \brief Signals that load data for a new phase is available
   *
   * For models that want to do pre-computation based on measured
   * loads before being asked to provide predictions from them
   *
   * This would typically be called by LBManager collectively inside
   * an epoch that can be used for global communication in advance of
   * any calls to getWork()
   */
  virtual void updateLoads(PhaseType last_completed_phase) { }

  /**
   * \brief Provide an estimate of the given object's load during a specified interval
   *
   * \param[in] object The object whose load is desired
   * \param[in] when The interval in which the estimated load is desired
   *
   * \return How much computation time the object is estimated to require
   */
  virtual TimeType getWork(ElementIDType object, PhaseOffset when) = 0;

  // Object enumeration, to abstract away access to the underlying structures from ProcStats
  ObjectIterator begin() { return ObjectIterator(proc_load_->back().begin()); }
  ObjectIterator end()   { return ObjectIterator(proc_load_->back().end()); }

  int getNumObjects() { return end() - begin(); }

protected:
  // Observer pointers to the underlying data. In operation, these would be owned by ProcStats
  std::vector<LoadMapType>         const* proc_load_;
  std::vector<SubphaseLoadMapType> const* proc_subphase_load_;
  std::vector<CommMapType>         const* proc_comm_;
}; // class LoadModel

}}}} // namespaces

#endif
