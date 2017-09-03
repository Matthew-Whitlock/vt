
#if ! defined __RUNTIME_TRANSPORT_SEQ_STATE__
#define __RUNTIME_TRANSPORT_SEQ_STATE__

#include "seq_common.h"
#include "seq_action.h"

namespace runtime { namespace seq {

template <typename MessageT, action_any_function_t<MessageT>* f>
struct SeqMsgState {
  using action_t = Action<MessageT>;

  template <typename T>
  using tag_container_t = std::unordered_map<tag_t, T>;

  using action_container_t = std::list<action_t>;
  using tagged_action_container_t = tag_container_t<action_container_t>;

  using msg_container_t = std::list<MessageT*>;
  using tagged_msg_container_t = tag_container_t<std::list<MessageT*>>;

  // waiting actions on matching message arrival
  static action_container_t seq_action;
  static tagged_action_container_t seq_action_tagged;

  // waiting messages on matching action arrival
  static msg_container_t seq_msg;
  static tagged_msg_container_t seq_msg_tagged;
};

template <typename MessageT, action_any_function_t<MessageT>* f>
using seq_state_t = SeqMsgState<MessageT, f>;

template <typename MessageT, action_any_function_t<MessageT>* f>
typename seq_state_t<MessageT, f>::action_container_t seq_state_t<MessageT, f>::seq_action;

template <typename MessageT, action_any_function_t<MessageT>* f>
typename seq_state_t<MessageT, f>::tagged_action_container_t seq_state_t<MessageT, f>::seq_action_tagged;

template <typename MessageT, action_any_function_t<MessageT>* f>
typename seq_state_t<MessageT, f>::msg_container_t seq_state_t<MessageT, f>::seq_msg;

template <typename MessageT, action_any_function_t<MessageT>* f>
typename seq_state_t<MessageT, f>::tagged_msg_container_t seq_state_t<MessageT, f>::seq_msg_tagged;

}} //end namespace runtime::seq

#endif /* __RUNTIME_TRANSPORT_SEQ_STATE__*/
