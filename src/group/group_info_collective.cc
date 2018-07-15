
#include "config.h"
#include "group/group_common.h"
#include "group/group_info_base.h"
#include "group/group_info_collective.h"
#include "group/group_collective.h"
#include "group/group_collective_msg.h"
#include "context/context.h"
#include "messaging/active.h"
#include "collective/tree/tree.h"
#include "collective/collective_alg.h"
#include "group/group_manager.h"

#include <memory>
#include <set>
#include <cstdlib>
#include <vector>
#include <list>
#include <algorithm>
#include <cassert>

namespace vt { namespace group {

void InfoColl::setupCollective() {
  auto const& group_ = getGroupID();

  assert(!collective_   && "Collective should not be initialized");

  if (!collective_) {
    collective_ = std::make_unique<InfoColl::GroupCollectiveType>();
  }

  auto const& in_group = is_in_group;
  auto const& parent = collective_->getInitialParent();
  auto const& children = collective_->getInitialChildren();

  this->coll_wait_count_ = children + 1;

  debug_print(
    group, node,
    "InfoColl::setupCollective: is_in_group={}, group_={:x}, parent={}, "
    "children={}, wait={}\n",
    is_in_group, group_, parent, children, coll_wait_count_
  );

  up_tree_cont_       = makeCollectiveContinuation(group_);
  down_tree_cont_     = theGroup()->nextCollectiveID();
  down_tree_fin_cont_ = theGroup()->nextCollectiveID();
  finalize_cont_      = theGroup()->nextCollectiveID();
  new_tree_cont_      = theGroup()->nextCollectiveID();

  theGroup()->registerContinuationT<GroupCollectiveMsg*>(
    down_tree_cont_,
    [group_](GroupCollectiveMsg* msg){
      auto iter = theGroup()->local_collective_group_info_.find(group_);
      assert(iter != theGroup()->local_collective_group_info_.end());
      iter->second->downTree(msg);
    }
  );
  theGroup()->registerContinuationT<GroupOnlyMsg*>(
    down_tree_fin_cont_,
    [group_](GroupOnlyMsg* msg){
      auto iter = theGroup()->local_collective_group_info_.find(group_);
      assert(iter != theGroup()->local_collective_group_info_.end());
      iter->second->downTreeFinished(msg);
    }
  );
  theGroup()->registerContinuationT<GroupOnlyMsg*>(
    finalize_cont_,
    [group_](GroupOnlyMsg* msg){
      auto iter = theGroup()->local_collective_group_info_.find(group_);
      assert(iter != theGroup()->local_collective_group_info_.end());
      iter->second->finalizeTree(msg);
    }
  );
  theGroup()->registerContinuationT<GroupOnlyMsg*>(
    new_tree_cont_,
    [group_](GroupOnlyMsg* msg){
      auto iter = theGroup()->local_collective_group_info_.find(group_);
      assert(iter != theGroup()->local_collective_group_info_.end());
      auto const& from = theMsg()->getFromNodeCurrentHandler();
      iter->second->newTree(from);
    }
  );

  debug_print(
    group, node,
    "InfoColl::setupCollective: is_in_group={}, parent={}: up tree\n",
    is_in_group, parent
  );

  if (collective_->getInitialChildren() == 0) {
    auto const& size = is_in_group ? 1 : 0;
    auto const& child = theContext()->getNode();
    auto msg = makeSharedMessage<GroupCollectiveMsg>(
      group_, up_tree_cont_, in_group, size, child
    );
    theMsg()->sendMsg<GroupCollectiveMsg,upHan>(parent, msg);
  }
}

void InfoColl::atRoot() {
  auto const& is_root = collective_->isInitialRoot();
  auto const& group = getGroupID();
  debug_print(
    group, node,
    "InfoColl::atRoot: is_in_group={}, group={:x}, root={}, children={}\n",
    is_in_group, group, is_root, collective_->span_children_.size()
  );
}

void InfoColl::upTree() {
  assert(
    msgs_.size() - extra_count_  == coll_wait_count_ - 1 && "Must be equal"
  );
  decltype(msgs_) msg_in_group = {};
  std::size_t subtree = 0;
  for (auto&& msg : msgs_) {
    if (msg->isInGroup()) {
      msg_in_group.push_back(msg);
      subtree += msg->getSubtreeSize();
    }
  }

  auto& span_children_ = collective_->span_children_;
  auto const& is_root = collective_->isInitialRoot();
  auto const& group = getGroupID();
  auto const& p = collective_->getInitialParent();
  auto const& op = up_tree_cont_;

  debug_print(
    group, node,
    "InfoColl::upTree: is_in_group={}, msg_in_group.size()={}, group={:x}, "
    "op={:x}, is_root={}\n",
    is_in_group, msg_in_group.size(), group, op, is_root
  );

  if (is_root) {
    for (auto&& msg : msg_in_group) {
      span_children_.push_back(msg->getChild());
    }
    in_phase_two_ = true;
    finalize();
    return;
  }

  if (is_in_group && (msg_in_group.size() == 2 || msg_in_group.size() == 0)) {
    /*
     *  Case where we have an approx. a balanced tree: send up the tree like
     *  usual
     */
    auto const& child = theContext()->getNode();
    auto const& size = subtree + 1;
    auto const& level =
      msg_in_group.size() == 2 ? msg_in_group[0]->getLevel() + 1 : 0;
    auto msg = makeSharedMessage<GroupCollectiveMsg>(
      group,op,is_in_group,size,child,level
    );
    theMsg()->sendMsg<GroupCollectiveMsg,upHan>(p, msg);

    for (auto&& msg : msg_in_group) {
      span_children_.push_back(msg->getChild());
    }
  } else if (!is_in_group && msg_in_group.size() < 3) {
    /*
     * Promote the non-null children to the child of the current parent
     * bypassing this node that is null: thus, forward the non-null's child's
     * message up the initial spanning tree
     */
    auto const& child = theContext()->getNode();
    auto msg = makeSharedMessage<GroupCollectiveMsg>(
      group,op,is_in_group,0,child,0,msg_in_group.size()
    );
    theMsg()->sendMsg<GroupCollectiveMsg,upHan>(p, msg);
    /*
     *  Forward all the children messages up the tree (up to 2 of them)
     */
    for (std::size_t i = 0; i < msg_in_group.size(); i++) {
      theMsg()->sendMsg<GroupCollectiveMsg,upHan>(p, msg_in_group[i]);
    }
  } else if (is_in_group && msg_in_group.size() == 1) {
    /*
     * In this case, we have two valid nodes in the tree (the current node and a
     * child of this current node); we should not make the child on this node a
     * child in the spanning tree because that will create a stick-like graph,
     * loosing efficiency!
     */
    auto const& child = theContext()->getNode();
    auto msg = makeSharedMessage<GroupCollectiveMsg>(
      group,op,is_in_group,1,child,0,1
    );
    theMsg()->sendMsg<GroupCollectiveMsg,upHan>(p, msg);
    theMsg()->sendMsg<GroupCollectiveMsg,upHan>(p, msg_in_group[0]);
  } else {
    assert(msg_in_group.size() > 2);

    std::vector<GroupCollectiveMsg*> msg_list;
    for (auto&& msg : msg_in_group) {
      debug_print(
        group, node,
        "InfoColl::upTree: inserting into set msg={}\n",
        print_ptr(msg)
      );
      msg_list.emplace_back(msg);
    }

    auto const& extra = msg_in_group.size() / 2;
    auto const& child = theContext()->getNode();
    auto msg = makeSharedMessage<GroupCollectiveMsg>(
      group,op,is_in_group,0,child,0,extra
    );
    theMsg()->sendMsg<GroupCollectiveMsg,upHan>(p, msg);

    debug_print(
      group, node,
      "InfoColl::upTree: msg_in_group.size()={}, msg_size.size()={}\n",
      msg_in_group.size(), msg_list.size()
    );

    std::sort(msg_list.begin(), msg_list.end(), GroupCollSort());

    auto iter = msg_list.rbegin();
    auto iter_end = msg_list.rend();
    NodeType* c = static_cast<NodeType*>(std::malloc(sizeof(NodeType) * extra));

    for (int i = 0; i < extra; i++) {
      c[i] = (*iter)->getChild();
      theMsg()->sendMsg<GroupCollectiveMsg,upHan>(p, *iter);
      iter++;
    }

    int32_t i = 0;
    while (iter != iter_end) {
      (*iter)->setOpID(down_tree_cont_);
      theMsg()->sendMsg<GroupCollectiveMsg,downHan>(c[i % extra],*iter);
      ++send_down_;
      ++iter;
    }

    std::free(c);
  }
}

RemoteOperationIDType InfoColl::makeCollectiveContinuation(
  GroupType const group_
) {
  auto const& id = theGroup()->nextCollectiveID();
  theGroup()->registerContinuationT<GroupCollectiveMsg*>(
    id, [group_](GroupCollectiveMsg* msg){
      auto iter = theGroup()->local_collective_group_info_.find(group_);
      assert(iter != theGroup()->local_collective_group_info_.end());
      iter->second->collectiveFn(msg);
    }
  );
  return id;
}

void InfoColl::collectiveFn(GroupCollectiveMsg* msg) {
  messageRef(msg);
  msgs_.push_back(msg);

  auto const& child = msg->getChild();
  auto const& children = collective_->getChildren();
  auto const& is_root = collective_->isInitialRoot();
  bool found = false;
  for (auto&& c : children) {
    if (child == c) {
      found = true;
      break;
    }
  }
  if (found) {
    arrived_count_++;
    extra_count_ += msg->getExtraNodes();
  } else {
    extra_arrived_count_++;
  }

  auto const& ready =
    coll_wait_count_ + extra_count_ == arrived_count_ + extra_arrived_count_ + 1;

  debug_print(
    group, node,
    "InfoColl::collectiveFn: group={:x}, arrived_count_={}, extra_count_={}, "
    "coll_wait_count_-1={}, ready={}\n",
    msg->getGroup(), arrived_count_, extra_count_, coll_wait_count_-1,
    ready
  );

  if (ready) {
    upTree();
    if (is_root) {
      atRoot();
    }
  }
}

/*static*/ void InfoColl::tree(GroupOnlyMsg* msg) {
  messageRef(msg);
  auto const& op_id = msg->getOpID();
  assert(op_id != no_op_id && "Must have valid op");
  theGroup()->triggerContinuationT<GroupOnlyMsg*>(op_id,msg);
}

/*static*/ void InfoColl::upHan(GroupCollectiveMsg* msg) {
  debug_print(
    group, node,
    "InfoColl::upHan: group={:x}, op={:x}, child={}, extra={}\n",
    msg->getGroup(), msg->getOpID(), msg->getChild(), msg->getExtraNodes()
  );

  messageRef(msg);
  auto const& op_id = msg->getOpID();
  assert(op_id != no_op_id && "Must have valid op");
  theGroup()->triggerContinuationT<GroupCollectiveMsg*>(op_id,msg);
  //messageDeref(msg);
}

void InfoColl::downTree(GroupCollectiveMsg* msg) {
  debug_print(
    group, node,
    "InfoColl::downTree: group={:x}, child={}\n",
    getGroupID(), msg->getChild()
  );

  auto const& from = theMsg()->getFromNodeCurrentHandler();
  assert(collective_ && "Must be valid");
  collective_->span_children_.push_back(msg->getChild());
  auto const& group_ = getGroupID();
  auto nmsg = makeSharedMessage<GroupOnlyMsg>(group_,down_tree_fin_cont_);
  theMsg()->sendMsg<GroupOnlyMsg,downFinishedHan>(from,nmsg);
}

void InfoColl::newTree(NodeType const& parent) {
  auto const& group_ = getGroupID();
  collective_->parent_ = parent;
  sendDownNewTree();
  assert(is_in_group && "Must be in group");
  auto const& is_root = parent == -1;
  collective_->span_   = std::make_unique<TreeType>(
    is_root, collective_->parent_, collective_->span_children_
  );
  collective_->reduce_ = std::make_unique<ReduceType>(
    group_, collective_->span_.get()
  );
  auto const& action = getAction();
  if (action) {
    action();
  }
}

void InfoColl::sendDownNewTree() {
  auto const& group_ = getGroupID();
  auto const& children = collective_->span_children_;
  for (auto&& c : children) {
    debug_print(
      group, node,
      "InfoColl::sendDownNewTree: group={:x}, sending to child={}\n",
      group_, c
    );
    auto msg = makeSharedMessage<GroupOnlyMsg>(group_,new_tree_cont_);
    theMsg()->sendMsg<GroupOnlyMsg,newTreeHan>(c,msg);
  }
}

void InfoColl::finalize() {
  auto const& group_ = getGroupID();

  char buf[256];
  buf[0] = '\0';
  int cur = 0;
  for (auto&& elm : collective_->span_children_) {
    cur += sprintf(buf + cur, "%d,", elm);
  }

  debug_print(
    group, node,
    "InfoColl::finalize: group={:x}, send_down_={}, send_down_finished_={}, "
    "children={}, in_phase_two_={}, in_group={}, children={}\n",
    group_, send_down_, send_down_finished_, collective_->span_children_.size(),
    in_phase_two_, is_in_group, buf
  );

  if (in_phase_two_ && send_down_finished_ == send_down_) {
    auto const& children = collective_->getChildren();
    for (auto&& c : children) {

      debug_print(
        group, node,
        "InfoColl::finalize: group={:x}, sending to child={}\n",
        group_, c
      );

      auto msg = makeSharedMessage<GroupOnlyMsg>(group_,finalize_cont_);
      theMsg()->sendMsg<GroupOnlyMsg,finalizeHan>(c,msg);
    }

    if (!is_in_group) {
      auto const& action = getAction();
      if (action) {
        action();
      }
    }

    auto const& root = 0;
    auto msg = makeSharedMessage<FinishedReduceMsg>(group_);
    theCollective()->reduce<
      FinishedReduceMsg,
      FinishedReduceMsg::msgHandler<
        FinishedReduceMsg,
        collective::PlusOp<collective::NoneType>,
        FinishedWork
      >
    >(root,msg);
  }
}

void InfoColl::finalizeTree(GroupOnlyMsg* msg) {
  debug_print(
    verbose, group, node,
    "InfoColl::finalizeTree: group={:x}\n",
    msg->getGroup()
  );
  in_phase_two_ = true;
  finalize();
}

void InfoColl::downTreeFinished(GroupOnlyMsg* msg) {
  send_down_finished_++;
  finalize();
}

/*static*/ void InfoColl::downHan(GroupCollectiveMsg* msg) {
  return upHan(msg);
}

/*static*/ void InfoColl::downFinishedHan(GroupOnlyMsg* msg) {
  return tree(msg);
}

/*static*/ void InfoColl::finalizeHan(GroupOnlyMsg* msg) {
  return tree(msg);
}

/*static*/ void InfoColl::newTreeHan(GroupOnlyMsg* msg) {
  return tree(msg);
}

InfoColl::ReducePtrType InfoColl::getReduce() const {
  return collective_->reduce_.get();
}

void FinishedWork::operator()(FinishedReduceMsg* msg) {
  debug_print(
    verbose, group, node,
    "FinishedWork: group={:x}\n", msg->getGroup()
  );
  auto iter = theGroup()->local_collective_group_info_.find(msg->getGroup());
  assert(
    iter != theGroup()->local_collective_group_info_.end() && "Must exist"
  );
  iter->second->newTree(-1);
}

}} /* end namespace vt::group */
