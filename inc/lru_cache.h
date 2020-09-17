/* 
 * File:   lru_cache.h
 * CODE ADAPTED FROM https://github.com/lamerman/cpp-lru-cache
 *
 * Created on September 16, 2020, 2:29 PM
 */

#ifndef LRU_CACHE_H
#define LRU_CACHE_H

#include <unordered_map>
#include <list>
#include <cstddef>
#include <stdexcept>

template<typename key_t, typename value_t>
class LRUCache {
public:
  typedef typename std::pair<key_t, value_t> key_value_pair_t;
  typedef typename std::list<key_value_pair_t>::iterator list_iterator_t;

  LRUCache(size_t max_size) :
    capacity_(max_size) {
    cache_.reserve(max_size);
  }

  void put(const key_t& key, const value_t& value) {
    auto it = cache_.find(key);
    lru_list_.push_front(key_value_pair_t(key, value));
    if (it != cache_.end()) {
      lru_list_.erase(it->second);
      cache_.erase(it);
    }
    cache_[key] = lru_list_.begin();

    if (cache_.size() > capacity_) {
      auto last = lru_list_.end();
      last--;
      cache_.erase(last->first);
      lru_list_.pop_back();
    }
  }

  const value_t& get(const key_t& key) {
    auto it = cache_.find(key);
    if (it == cache_.end()) {
      throw std::range_error("There is no such key in cache");
    } else {
      lru_list_.splice(lru_list_.begin(), lru_list_, it->second);
      return it->second->second;
    }
  }

  bool exists(const key_t& key) const {
    return cache_.find(key) != cache_.end();
  }

  size_t size() const {
    return cache_.size();
  }

  void clear() {
    cache_.clear();
    lru_list_.clear();
  }

private:
  std::list<key_value_pair_t> lru_list_;
  std::unordered_map<key_t, list_iterator_t> cache_;
  size_t capacity_;
};

#endif /* LRU_CACHE_H */

