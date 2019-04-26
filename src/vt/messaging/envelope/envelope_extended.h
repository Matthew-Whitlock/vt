/*
//@HEADER
// ************************************************************************
//
//                          envelope_extended.h
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

#if !defined INCLUDED_MESSAGING_ENVELOPE_ENVELOPE_EXTENDED_H
#define INCLUDED_MESSAGING_ENVELOPE_ENVELOPE_EXTENDED_H

#include "vt/config.h"
#include "vt/messaging/envelope/envelope_type.h"
#include "vt/messaging/envelope/envelope_base.h"

#include <type_traits>

namespace vt { namespace messaging {

struct EpochActiveEnvelope {
  using isByteCopyable = std::true_type;

  ActiveEnvelope env;
  EpochType epoch : epoch_num_bits;
};

struct TagActiveEnvelope {
  using isByteCopyable = std::true_type;

  ActiveEnvelope env;
  TagType tag     : tag_num_bits;
};

struct EpochTagActiveEnvelope {
  using isByteCopyable = std::true_type;

  ActiveEnvelope env;
  EpochType epoch : epoch_num_bits;
  TagType tag     : tag_num_bits;
};

}} //end namespace vt::messaging

namespace vt {

using EpochEnvelope    = messaging::EpochActiveEnvelope;
using TagEnvelope      = messaging::TagActiveEnvelope;
using EpochTagEnvelope = messaging::EpochTagActiveEnvelope;

static_assert(std::is_pod<EpochEnvelope>(),    "EpochEnvelope must be POD");
static_assert(std::is_pod<TagEnvelope>(),      "TagEnvelope must be POD");
static_assert(std::is_pod<EpochTagEnvelope>(), "EpochTagEnvelope must be POD");
static_assert(
  std::is_trivially_destructible<EpochEnvelope>(),
  "EpochEnvelope must be trivial"
);
static_assert(
  std::is_trivially_destructible<TagEnvelope>(),
  "TagEnvelope must be trivial"
);
static_assert(
  std::is_trivially_destructible<EpochTagEnvelope>(),
  "EpochTagEnvelope must be trivial"
);

} /* end namespace vt */

#include "vt/messaging/envelope/envelope_extended.impl.h"

#endif /*INCLUDED_MESSAGING_ENVELOPE_ENVELOPE_EXTENDED_H*/
