/*
//@HEADER
// ************************************************************************
//
//                          pending_send.h
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

#if !defined INCLUDED_VT_MESSAGING_PENDING_SEND_H
#define INCLUDED_VT_MESSAGING_PENDING_SEND_H

#include "vt/config.h"
#include "vt/messaging/message.h"

#include <functional>
#include <cstddef>

namespace vt { namespace messaging {

struct PendingSend final {
  using SendActionType = std::function<void(MsgVirtualPtr<BaseMsgType>)>;

  PendingSend(MsgSharedPtr<BaseMsgType> const& in_msg, ByteType const& in_msg_size)
    : msg_(in_msg.template toVirtual<BaseMsgType>())
    , msg_size_(in_msg_size)
  { }
  template <typename MsgT>
  PendingSend(MsgSharedPtr<MsgT> in_msg, SendActionType const& in_action)
    : msg_(in_msg.template toVirtual<BaseMsgType>())
    , msg_size_(sizeof(MsgT))
    , send_action_(in_action)
  { }

  explicit PendingSend(std::nullptr_t) { }
  PendingSend(PendingSend&& in)
    : msg_(std::move(in.msg_)),
      msg_size_(std::move(in.msg_size_)),
      send_action_(std::move(in.send_action_))
  {
    in.msg_ = nullptr;
    in.send_action_ = nullptr;
  }
  PendingSend(const PendingSend&) = delete;
  PendingSend& operator=(PendingSend&& in) = delete;
  PendingSend& operator=(PendingSend& in) = delete;

  ~PendingSend() { release(); }

  void release() {
    if (msg_ != nullptr || send_action_ != nullptr) {
      sendMsg();
    }
  }

private:
  // Send the message saved directly or trigger the lambda for
  // specialized sends from the pending holder
  void sendMsg();

private:
  MsgVirtualPtr<BaseMsgType> msg_ = nullptr;
  ByteType msg_size_ = no_byte;
  SendActionType send_action_ = nullptr;
};

}} /* end namespace vt::messaging */

#endif /*INCLUDED_VT_MESSAGING_PENDING_SEND_H*/
