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

  Type& insert(const Type& item) {
    auto result = base_t::insert({item.getName(), item});
    if (result.second) { byIndex_.push_back(&result.first->second); }
    return result.first->second;
  }

  std::vector<const Type*> toVector() const {
    return std::vector<const Type*>(byIndex_.begin(), byIndex_.end());
  }

  const Type* atByIndex(size_t index) const { return byIndex_.at(index); }

  size_t size() const { return byIndex_.size(); }

 protected:
  std::vector<Type*> byIndex_;
};

#endif /* COLLECTION_H */

