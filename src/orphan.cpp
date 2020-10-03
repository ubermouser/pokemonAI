#include "../inc/orphan.h"

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
    const std::string& type,
    int verbosity_level) {
  if (orphans.size() > 0)
  {
    std::cerr << "WAR " << __FILE__ << "." << __LINE__ <<
      ": \"" << source <<
      "\" - " << orphans.size() << " Orphaned " << categoryName << "!\n";
    if (verbose >= verbosity_level)
    {
      for (auto& orphan: orphans)
      {
        std::cout << "\tOrphaned " << type << " \"" << orphan << "\"\n";
      }
    }
  }
}


void orphan::Orphanage::printAllOrphans(
      const std::string& source,
      const std::string& prefix,
      int verbosity_level) const {
  auto formatter = boost::format("%s-%s");

  // print mismatched pokemon
  printOrphans(
      pokemon, source, (formatter % prefix % "pokemon").str(), "pokemon", verbosity_level);

  // print mismatched items
  printOrphans(
      items, source, (formatter % prefix % "items").str(), "item", verbosity_level);

  // print mismatched abilities
  printOrphans(
      abilities, source, (formatter % prefix % "abilities").str(), "ability", verbosity_level);

  // print mismatched natures
  printOrphans(
      natures, source, (formatter % prefix % "natures").str(), "nature", verbosity_level);

  // print mismatched moves
  printOrphans(
      moves, source, (formatter % prefix % "moves").str(), "move", verbosity_level);
}
