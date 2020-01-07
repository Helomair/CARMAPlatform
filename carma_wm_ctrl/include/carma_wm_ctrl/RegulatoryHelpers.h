#pragma once
#include <lanelet2_core/primitives/RegulatoryElement.h> 
#include <boost/algorithm/string.hpp> 
#include <vector>
#include <unordered_set>

namespace lanelet {
  bool setContainsParticipant(const std::unordered_set<std::string>& set, const std::string& participant) {
    std::vector<std::string> strs;
    boost::split(strs,participant,boost::is_any_of(":"));
    std::string current_string;
    bool first = true;
    for (auto participant_class : strs) {
      if (first) {
        current_string = participant_class;
        first =  false;
      } else {
        current_string += ":" + participant_class;
      }
      if (set.find(current_string) != set.end())
      {
        return true;
      }
    }

    return false;
  }
}