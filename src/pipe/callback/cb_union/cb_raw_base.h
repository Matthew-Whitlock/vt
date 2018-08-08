
#if !defined INCLUDED_PIPE_CALLBACK_CB_UNION_CB_RAW_BASE_H
#define INCLUDED_PIPE_CALLBACK_CB_UNION_CB_RAW_BASE_H

#include "config.h"
#include "pipe/callback/cb_union/cb_raw.h"

#include <cassert>

namespace vt { namespace pipe { namespace callback { namespace cbunion {

static struct RawAnonTagType        { } RawAnonTag        { };
static struct RawSendMsgTagType     { } RawSendMsgTag     { };
static struct RawBcastMsgTagType    { } RawBcastMsgTag    { };
static struct RawSendColMsgTagType  { } RawSendColMsgTag  { };
static struct RawBcastColMsgTagType { } RawBcastColMsgTag { };

struct CallbackRawBaseSingle {

  CallbackRawBaseSingle() = default;
  CallbackRawBaseSingle(
    RawSendMsgTagType, PipeType const& in_pipe, HandlerType const& in_handler,
    NodeType const& in_node
  );
  CallbackRawBaseSingle(
    RawBcastMsgTagType, PipeType const& in_pipe, HandlerType const& in_handler,
    bool const& in_inc
  );
  CallbackRawBaseSingle(RawAnonTagType, PipeType const& in_pipe);
  CallbackRawBaseSingle(RawSendColMsgTagType, PipeType const& in_pipe);
  CallbackRawBaseSingle(RawBcastColMsgTagType, PipeType const& in_pipe);

  template <typename MsgT>
  void send(MsgT* msg);

  void send();

  template <typename SerializerT>
  void serialize(SerializerT& s);

  PipeType getPipe() const { return pipe_; }

private:
  PipeType pipe_ = no_pipe;
  GeneralCallback cb_;
};

template <typename MsgT>
struct CallbackTyped : CallbackRawBaseSingle {
  using VoidSigType   = signal::SigVoidType;
  template <typename T, typename U=void>
  using IsVoidType    = std::enable_if_t<std::is_same<T,VoidSigType>::value,U>;
  template <typename T, typename U=void>
  using IsNotVoidType = std::enable_if_t<!std::is_same<T,VoidSigType>::value,U>;

  CallbackTyped() = default;
  CallbackTyped(CallbackTyped const&) = default;
  CallbackTyped(CallbackTyped&&) = default;
  CallbackTyped& operator=(CallbackTyped const&) = default;

  // Conversion operators to typed from untyped
  CallbackTyped(CallbackRawBaseSingle const& other) {
    pipe_ = other.pipe_;
    cb_   = other.cb_;
  }
  CallbackTyped(CallbackRawBaseSingle&& other) {
    pipe_ = std::move(other.pipe_);
    cb_   = std::move(other.cb_);
  }

  template <typename MsgU>
  IsNotVoidType<MsgU> send(MsgU* m) {
    static_assert(std::is_same<MsgT,MsgU>::value, "Required exact type match");
    CallbackRawBaseSingle::send<MsgU>(m);
  }

  template <typename T=void, typename=IsVoidType<MsgT,T>>
  void send() {
    CallbackRawBaseSingle::send();
  }

  template <typename SerializerT>
  void serialize(SerializerT& s) {
    CallbackRawBaseSingle::serialize(s);
  }

};

}}}} /* end namespace vt::pipe::callback::cbunion */

namespace vt {

template <typename MsgT>
using Callback = pipe::callback::cbunion::CallbackTyped<MsgT>;

using CallbackU = pipe::callback::cbunion::CallbackRawBaseSingle;

} /* end namespace vt */

#include "pipe/callback/cb_union/cb_raw_base.impl.h"

#endif /*INCLUDED_PIPE_CALLBACK_CB_UNION_CB_RAW_BASE_H*/
