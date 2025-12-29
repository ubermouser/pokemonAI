#include "pokemonai/orphan.h"

#include <iostream>

#include <boost/algorithm/string/case_conv.hpp>
#include <boost/format.hpp>

std::string orphan::lowerCase(const std::string& source) {
  std::string result(source);
  boost::to_lower(result);
  return result;
}


void orphan::printOrphans(
    const OrphanSet& orphans,
    const std::string& source,
    const std::string& categoryName,
    const std::string& type) {
  if (orphans.size() > 0)
  {
    SPDLOG_WARN("\"{}\" - {} Orphaned {}!", source, orphans.size(), categoryName);
    if (SPDLOG_ACTIVE_LEVEL <= SPDLOG_LEVEL_DEBUG)
    {
      for (auto& orphan: orphans)
      {
        SPDLOG_DEBUG("\tOrphaned {} \"{}\"", type, orphan);
      }
    }
  }
}


void orphan::Orphanage::printAllOrphans(
      const std::string& source,
      const std::string& prefix) const {
  auto formatter = boost::format("%s-%s");

  // print mismatched pokemon
  printOrphans(
      pokemon, source, (formatter % prefix % "pokemon").str(), "pokemon");

  // print mismatched items
  printOrphans(
      items, source, (formatter % prefix % "items").str(), "item");

  // print mismatched abilities
  printOrphans(
      abilities, source, (formatter % prefix % "abilities").str(), "ability");

  // print mismatched natures
  printOrphans(
      natures, source, (formatter % prefix % "natures").str(), "nature");

  // print mismatched moves
  printOrphans(
      moves, source, (formatter % prefix % "moves").str(), "move");
}
