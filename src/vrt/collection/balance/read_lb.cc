
#include "config.h"
#include "context/context.h"
#include "vrt/collection/balance/read_lb.h"

#include <string>
#include <fstream>
#include <cassert>
#include <cctype>
#include <cmath>

namespace vt { namespace vrt { namespace collection { namespace balance {

/*static*/ std::string ReadLBSpec::filename = {};
/*static*/ SpecIndex ReadLBSpec::num_entries_ = 0;
/*static*/ std::unordered_map<SpecIndex,SpecEntry> ReadLBSpec::spec_ = {};
/*static*/ bool ReadLBSpec::has_spec_ = false;
/*static*/ bool ReadLBSpec::read_complete_ = false;

/*static*/ bool ReadLBSpec::openFile(std::string const name) {
  std::ifstream file(name);
  filename = name;
  has_spec_ = file.good();
  return file.good();
}

/*static*/ LBType ReadLBSpec::getLB(SpecIndex const& idx) {
  auto const lb = entry(idx);
  if (lb) {
    return lb->getLB();
  } else {
    return LBType::NoLB;
  }
}

/*static*/ SpecEntry const* ReadLBSpec::entry(SpecIndex const& idx) {
  auto spec_iter = spec_.find(idx);
  ::fmt::print(
    "idx={},idx-mod={},found={},size={},num_entries={}\n",
    idx,
    idx % num_entries_,
    spec_iter != spec_.end(),
    spec_.size(),
    num_entries_
  );
  if (spec_iter == spec_.end()) {
    spec_iter = spec_.find(idx % num_entries_);
    if (spec_iter != spec_.end()) {
      return &spec_iter->second;
    }
  } else {
    return &spec_iter->second;
  }
  return nullptr;
}

/*static*/ void ReadLBSpec::readFile() {
  if (read_complete_ || !has_spec_) {
    return;
  }

  std::ifstream file(filename);
  assert(file.good() && "must be valid");

  int64_t max_entry = 0;
  std::string cur_line;
  while (!file.eof()) {
    double lb_min = 0.0f, lb_max = 0.0f;
    int64_t lb_iter = -1;
    std::string lb_name;

    if (std::isdigit(file.peek())) {
      file >> lb_iter >> lb_name;
      max_entry = std::max(max_entry, lb_iter);
    } else {
      file >> lb_name;
      lb_iter = max_entry;
      max_entry++;
    }
    if (file.peek() != '\n') {
      file >> lb_min;
    }
    if (file.peek() != '\n') {
      file >> lb_max;
    }

    if (lb_name != "") {
      auto spec_index = static_cast<SpecIndex>(lb_iter);
      spec_.emplace(
        std::piecewise_construct,
        std::forward_as_tuple(spec_index),
        std::forward_as_tuple(SpecEntry{spec_index,lb_name,lb_min,lb_max})
      );
      ::fmt::print(
        "{}: insert: idx={},size={},name={},min={},max={}\n",
        theContext()->getNode(),
        spec_index,
        spec_.size(),
        lb_name,
        lb_min,
        lb_max
      );
    }
  }

  num_entries_ = spec_.size();

  for (auto&& elm : spec_) {
    num_entries_ = std::max(num_entries_, elm.second.getIdx());
  }

  read_complete_ = true;
}

}}}} /* end namespace vt::vrt::collection::balance */
