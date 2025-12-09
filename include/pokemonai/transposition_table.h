#ifndef TRANSPOSITION_TABLE_H
#define TRANSPOSITION_TABLE_H

#include "evaluator.h"
#include "lru_cache.h"


class TranspositionTable: public LRUCache<uint64_t, EvalResult> {
public:
  using base_t = LRUCache<uint64_t, EvalResult>;
  using base_t::base_t;
};

#endif
