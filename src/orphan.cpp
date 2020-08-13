#include "../inc/orphan.h"

#include <iostream>

#include <boost/algorithm/string/case_conv.hpp>

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
