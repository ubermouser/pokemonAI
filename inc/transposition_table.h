#ifndef TRANSPOSITION_TABLE_H
#define TRANSPOSITION_TABLE_H

#include "../inc/pkai.h"

#include <stdint.h>
#include <boost/smart_ptr/detail/spinlock.hpp>
#include <boost/thread/locks.hpp>
#include <boost/array.hpp>
#include <boost/static_assert.hpp>
#include <vector>
//#include <math.h>

typedef boost::detail::spinlock spinlock;

#include "fp_compare.h"

#ifdef _HISTORESIGNATURE
#include "environment_volatile.h"
#endif

union environment_possible;

#define ISOLD_SHIFT 7
#define STATUS_SHIFT 5
#define ISOLD_MASK 0x80
#define STATUS_MASK 0x60
#define DEPTH_MASK 0x1f
#define HASH_MASK 0x000000ffffffffff
#define HASH_SHIFT 24
#define NUMSPINLOCKS 100

union table_entry
{
  struct
  {
    /* the lowest bound on this ply's fitness */
    fixType lbFitness;

    /* amount of the ply completed (when equal to 1, lbFitness == ubFitness == fitness) */
    fixType probability;

    /* 40 bits of the 64 bit signature of this position's internal state. */
    boost::array<uint8_t, 5> hash;

    /* the best evaluated agent move as of the current state */
    int8_t bestAgentMove;

    /* the best evaluated other move as of the current state */
    int8_t bestOtherMove;

    /* depth this node was evaluated to */
    uint8_t depth : 5;

    /* status of this move - if it was a leaf node, on the horizon, or is a terminal node */
    uint8_t status : 2;

    /* is this node currently non-empty? Does it contain a useful transposition? */
    uint8_t inUse : 1;
  }data;
  uint64_t raw[2];

  static table_entry createEmpty();

  static table_entry create(const environment_possible& _element, 
  int8_t _bestAgentMove, 
  int8_t _bestOtherMove, 
  uint8_t _depth, 
  fpType _lbFitness, 
  fpType _probability, 
  uint8_t _status);

  void delEntry();

  void setHashPart(uint64_t cHash)
  {
    // modify hash, agent, other, and status as if they were one 8 byte long variable
    // WARNING! ENDIANNESS IS A PROBLEM HERE!
    uint64_t& partialHash = (uint64_t&) *data.hash.data();
    // format hash:
    cHash = cHash >> HASH_SHIFT;
    // clear area:
    partialHash = partialHash & ~(HASH_MASK);
    // add new hash:
    partialHash = partialHash | (cHash & HASH_MASK);
  };

  uint64_t getHashPart() const
  {
    uint64_t& partialHash = (uint64_t&) *data.hash.data();
    return (partialHash & HASH_MASK);
  };

  uint32_t getStatus() const
  {
    return data.status;
  };

  uint8_t getDepth() const
  {
    return data.depth;
  };

  int8_t getAgentMove() const
  {
    return data.bestAgentMove;
  };

  int8_t getOtherMove() const
  {
    return data.bestOtherMove;
  };

  bool inUse() const
  {
    return data.inUse>0;
  };

  fpType getlbFitness() const
  {
    return data.lbFitness.to_double();
  };

  fpType getProbability() const
  {
    return data.probability.to_double();
  };

  void getValues(
    int8_t& _bestAgentMove, 
    int8_t& _bestOtherMove, 
    uint8_t& _depth, 
    fpType& _lbFitness, 
    fpType& _probability, 
    uint8_t& _status) const
  {
    // assign passed in variables to their counterparts
    _bestAgentMove = data.bestAgentMove;
    _bestOtherMove = data.bestOtherMove;
    _depth = data.depth;
    _lbFitness = data.lbFitness.to_double();
    _probability = data.probability.to_double();
    _status = data.status;
  };

  void getAgentValues(int8_t& _bestAgentMove, 
    int8_t& _bestOtherMove) const
  {
    // assign passed in variables to their counterparts
    _bestAgentMove = data.bestAgentMove;
    _bestOtherMove = data.bestOtherMove;
  };

  bool update(const table_entry& other);

  bool update(int8_t _bestAgentMove, 
    int8_t _bestOtherMove, 
    uint8_t _depth, 
    fpType _lbFitness, 
    fpType _probability, 
    uint8_t _status = NODET_FULLEVAL);

bool operator==(const table_entry& other) const
  {
    if (getHashPart() == other.getHashPart())
    {
      return true;
    }
    else
    {
      return false;
    }
  };


  bool operator!=(const table_entry& other) const
  {
    return !(*this == other);
  };

  bool operator==(const environment_possible& other) const;

  bool operator!=(const environment_possible& other) const
  {
    return !(*this == other);
  };

  bool operator<=(const table_entry& other)
  {
    uint8_t otherDepth = other.getDepth();
    uint8_t thisDepth = getDepth();
    if (thisDepth > otherDepth)
    {
      return false;
    }
    else if ((thisDepth == otherDepth) && (data.probability > other.data.probability))
    {
      return false;
    }
    
    return true;
  };
};

class transposition_table
{
private:
  static table_entry entry_deleted;

  /* locked when the table needs to be grown or shrunk */
  boost::array<spinlock, NUMSPINLOCKS> locks;

  /* heap allocated table containing pointers to all valid hash table entries */
  std::vector<table_entry> hashTable;

  /* number of bins currently in the table */
  size_t numBins;

  /* number of bits (of the hash) delegated to the table and not the table entries */
  size_t numBits;

  /* maximum number of table_entries a single bin may hold */
  size_t binSize;

  /* number of elements currently in the table */
  size_t numElements;

  /* maximum number of elements the table may hold */
  size_t maxElements;

#ifdef _HTCOLLECTSTATISTICS
  /* number of probes recorded in all searches total */
  uint64_t totalProbes;

  /* number of stores recorded total */
  uint64_t totalStores;

  /* number of probe hits recorded in all searches total. numProbes - numProbeHits = numProbeMisses */
  uint64_t totalProbeHits;
   
  /* number of fully evaluated nodes in table */
  uint64_t numFullyEvaluatedNodes;
   
  /* number of partially evaluated nodes in table */
  uint64_t numPartiallyEvaluatedNodes;
#endif

  /* hash bitmask for separating the index from the key */
  uint64_t hashBitmask;

  uint64_t h(uint64_t value) const
  {
    return (value & hashBitmask) * binSize;
  };

  uint64_t hbin(uint64_t value) const
  {
    return (value & hashBitmask) % NUMSPINLOCKS;
  };

  uint64_t h2(uint64_t value) const
  {
    return value & (~hashBitmask);
  };

public:
  transposition_table(size_t _numBits, size_t _binSize)
    : locks(), 
    hashTable(), 
    numBins((((size_t)1)<<_numBits)),
    numBits(_numBits),
    binSize(_binSize),
    numElements(0), 
    maxElements((((size_t)1)<<_numBits) * _binSize), 
#ifdef _HTCOLLECTSTATISTICS
    totalProbes(0),
    totalStores(0),
    totalProbeHits(0),
    numFullyEvaluatedNodes(0),
    numPartiallyEvaluatedNodes(0),
#endif
    hashBitmask((1<<_numBits) - 1)
  {
    assert(_numBits < 64);
    entry_deleted.delEntry();
    // set entire hashtable to vector of empty table entries
    hashTable.resize(maxElements, entry_deleted);
  }

  ~transposition_table();

  fpType load() const
  {
    return ((fpType) numElements / (fpType) maxElements) * 100.0;
  };

#ifdef _HTCOLLECTSTATISTICS
  /* reset all _HTCOLLECTSTATISTICS counters */
  void resetStatistics();

  fpType percentProbeHits() const
  {
    return ((fpType) totalProbeHits / (fpType) totalProbes) * 100.0;
  };
#endif

  size_t deleteOld();

  void clear();

  /* simply returns if a given element is stored within the transposition table, does not attempt to return any values */
  table_entry findElement(const environment_possible& _element);

  /* attempts to find the element _element, returning the values from its entry in the hash table, or FALSE if it does not exist. */
  table_entry* findElement_nonlocking(const environment_possible& _element);

  /* populates a table entry with _element and the given values. Returns FALSE if an insertion was impossible */
  bool updateElement(
    const environment_possible& _element, 
    int8_t _bestAgentMove, 
    int8_t _bestOtherMove, 
    uint8_t _depth, 
    fpType _lbfitness, 
    fpType _probability, 
    uint8_t _status = NODET_FULLEVAL);

  /* inserts a table_entry into the hash table. If the number of probes is sufficiently large, will fail */
  bool updateElement(const table_entry& cEntry, uint64_t hash);

  /* how many elements are currently in the hash table. May not expand past capacity, and ideally will not near 80% of capacity. */
  size_t size() const { return numElements; } ;

  /* maximum number of elements the hash table may hold. Performance when near the capacity (>80%) will degrade dramatically. */
  size_t capacity() const { return maxElements; };

  /* upper bound number of probes the hash table will perform at any given point before determining that an element doesn't exist */
  size_t getBinSize() const { return binSize; };
  
  /* number of bins within the table. Number of elements is equivalent to numBins * binSize. */
  size_t getNumBins() const { return numBins; };

  /* number of bits (of the hash) delegated to the table and not the table entries */
  size_t getNumBits() const { return numBits; };

  /* resizes the array if _numBins or _binSize are different from maxElements.
  In its current state, will set the refcount of every node in the hash table to 0. */
  void resize(size_t _numBins, size_t _binSize);

};

#endif
