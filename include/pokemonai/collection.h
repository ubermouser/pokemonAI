#ifndef COLLECTION_H
#define COLLECTION_H

#include <vector>
#include <unordered_map>

/*template<class Nameable>
struct CollectionHasher {
  size_t operator()(const Nameable& object) {
    return std::hash<std::string>{}(object.getName());
  }
};*/


template<class Type>
class Collection : public std::unordered_map<std::string, Type> {
public:
  using base_t = std::unordered_map<std::string, Type>;

  std::vector<const Type*> toVector() const {
    std::vector<const Type*> result; result.reserve(base_t::size());
    for (auto& pair : *this) {
      result.push_back(&pair.second);
    }

    return result;
  }
};

#endif /* COLLECTION_H */

