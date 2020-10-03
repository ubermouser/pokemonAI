
#ifndef ORPHAN_H
#define ORPHAN_H

#include "../inc/pkai.h"

#include <unordered_set>
#include <string>

#include "../inc/collection.h"

namespace orphan
{
  using OrphanSet = std::unordered_set<std::string>;

  struct Orphanage {
    OrphanSet pokemon;
    OrphanSet items;
    OrphanSet abilities;
    OrphanSet natures;
    OrphanSet moves;

    void printAllOrphans(
      const std::string& source,
      const std::string& prefix,
      int verbosity_level = 5) const;
  };

  PKAISHARED std::string lowerCase(const std::string& source);

  PKAISHARED void printOrphans(
      const OrphanSet& orphans,
      const std::string& source,
      const std::string& categoryName,
      const std::string& type,
      int verbosity_level = 5);

  template<class ptrType>
  static const ptrType* orphanCheck(
      const Collection<ptrType>& library,
      const std::string& name_,
      OrphanSet* orphans = NULL) {
    // check lowercase strings only
    std::string name = lowerCase(name_);

    // if does not exist within the library:
    auto iterator = library.find(name);
    if (iterator == library.cend()) {
      // add string to the orphan set and return NULL
      if (orphans != NULL) {
        orphans->insert(name);
      }
      return NULL;
    }

    // if it does exist, return a pointer
    return &iterator->second;
  }

  template<class ptrType>
  static ptrType* orphanCheck(
      Collection<ptrType>& library,
      const std::string& name_,
      OrphanSet* orphans = NULL) {
    return const_cast<ptrType*>(orphanCheck(
        const_cast<const Collection<ptrType>&>(library),
        name_,
        orphans));
  }
};

#endif /* ORPHAN_H */