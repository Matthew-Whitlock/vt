/*
//@HEADER
// *****************************************************************************
//
//                                  fntraits.h
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

#if !defined INCLUDED_VT_UTILS_FNTRAITS_FNTRAITS_H
#define INCLUDED_VT_UTILS_FNTRAITS_FNTRAITS_H

namespace vt {

struct NoMsg {};

template <typename enabled, typename ObjT, typename... Args>
struct ObjFuncTraits;

template <typename ObjT, typename Return, typename Msg>
struct ObjFuncTraits<
  std::enable_if_t<std::is_convertible<Msg, vt::Message>::value>,
  ObjT,
  Return(*)(ObjT*, Msg*)
> {
  static constexpr bool is_member = false;
  using MsgT = Msg;
  using ReturnT = Return;
};

template <typename ObjT, typename Return>
struct ObjFuncTraits<
  std::enable_if_t<std::is_same_v<void, void>>,
  ObjT,
  Return(*)(ObjT*)
> {
  static constexpr bool is_member = false;
  using MsgT = NoMsg;
  using ReturnT = Return;
  using TupleType = std::tuple<>;
};

template <typename ObjT, typename Return, typename Arg, typename... Args>
struct ObjFuncTraits<
  std::enable_if_t<not std::is_convertible<Arg, vt::Message*>::value>,
  ObjT,
  Return(*)(ObjT*, Arg, Args...)
> {
  static constexpr bool is_member = false;
  using MsgT = NoMsg;
  using TupleType = std::tuple<std::decay_t<Arg>, std::decay_t<Args>...>;
  using ReturnT = Return;
};

template <typename ObjT, typename Return, typename Msg>
struct ObjFuncTraits<
  std::enable_if_t<std::is_convertible<Msg, vt::Message>::value>,
  ObjT,
  Return(ObjT::*)(Msg*)
> {
  static constexpr bool is_member = true;
  using MsgT = Msg;
  using ReturnT = Return;
};

template <typename ObjT, typename Return>
struct ObjFuncTraits<
  std::enable_if_t<std::is_same_v<void, void>>,
  ObjT,
  Return(ObjT::*)()
> {
  static constexpr bool is_member = true;
  using MsgT = NoMsg;
  using TupleType = std::tuple<>;
  using ReturnT = Return;
};

template <typename ObjT, typename Return, typename Arg, typename... Args>
struct ObjFuncTraits<
  std::enable_if_t<not std::is_convertible<Arg, vt::Message*>::value>,
  ObjT,
  Return(ObjT::*)(Arg, Args...)
> {
  static constexpr bool is_member = true;
  using MsgT = NoMsg;
  using TupleType = std::tuple<std::decay_t<Arg>, std::decay_t<Args>...>;
  using ReturnT = Return;
};

} /* end namespace vt */

#endif /*INCLUDED_VT_UTILS_FNTRAITS_FNTRAITS_H*/
