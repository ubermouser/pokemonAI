//#define PKAI_IMPORT
#include "../inc/transposition_table.h"

#include <cstring>

#include "../inc/environment_possible.h"

BOOST_STATIC_ASSERT(sizeof(table_entry) == (sizeof(uint64_t)*2));

table_entry transposition_table::entry_deleted = table_entry();





table_entry table_entry::create(const EnvironmentPossible& _element, 
  int8_t _bestAgentMove, 
  int8_t _bestOtherMove, 
  uint8_t _depth, 
  fpType _lbFitness, 
  fpType _probability, 
  uint8_t _status)
{
  table_entry result = 
  {{ 
    fixedpoint::create<30>(_lbFitness),
    fixedpoint::create<30>(_probability),
    std::array<uint8_t, 5>(),
    _bestAgentMove, 
    _bestOtherMove, 
    _depth, 
    _status, 
    1
   }};
  result.setHashPart(_element.getHash());
  return result;
};




table_entry table_entry::createEmpty()
{
  table_entry result;
  // zero result:
  result.delEntry();
  // return:
  return result;
}





void table_entry::delEntry()
{
  // no guarantee element is aligned (WHY NOT!?)
  raw[0] = 0;
  raw[1] = 0;
};







bool table_entry::operator==(const EnvironmentPossible& other) const
{
  return getHashPart() == ((other.getHash() >> HASH_SHIFT) & HASH_MASK);
};





bool table_entry::update(
  int8_t _bestAgentMove, 
  int8_t _bestOtherMove, 
  uint8_t _depth, 
  fpType _lbFitness, 
  fpType _probability, 
  uint8_t _status)
{
  // only perform an update if the guess we just evaluated is better than the current guess
  uint8_t cDepth = data.depth;
  fpType cProbability = data.probability.to_double();
  //fpType cProbability = (fpType) probability;

  // if node was fully evaluated before:
  if (((data.status & 1) == NODET_FULLEVAL) && mostlyEQ(cProbability, 1.0)) { return false; }

  // if node's depth is greater than evaluated depth:
  if (cDepth > _depth) { return false; }

  // if depths are equal BUT the accuracy of the node's depth is greater:
  if (cDepth == _depth && mostlyLTE(_probability, cProbability)) { return false; }

  // acceptable cases:
  //	entry not fully evaluated
  //	new depth greater than old depth
  //	new probability greater than old probability

  // update variables
  data.bestAgentMove = _bestAgentMove;
  data.bestOtherMove = _bestOtherMove;
  data.depth = _depth;
  data.status = _status;
  data.inUse = 1;
  data.lbFitness = fixedpoint::create<30>(_lbFitness);
  data.probability = fixedpoint::create<30>(_probability);

  return true;
}





bool table_entry::update(const table_entry& other)
{
  return update(other.getAgentMove(), other.getOtherMove(), other.getDepth(), other.getlbFitness(), other.getProbability(), other.getStatus());
}





bool transposition_table::updateElement(
  const EnvironmentPossible& _element, 
  int8_t _bestAgentMove, 
  int8_t _bestOtherMove, 
  uint8_t _depth, 
  fpType _lbFitness, 
  fpType _probability, 
  uint8_t _status)
{
  // implicitly generate subhash key from _element signature, or load them from previously generated _element hash
  table_entry cEntry = table_entry::create(
    _element, 
    _bestAgentMove, 
    _bestOtherMove, 
    _depth,
    _lbFitness,
    _probability,
    _status);

  return updateElement(cEntry, _element.getHash());
}





bool transposition_table::updateElement(const table_entry& cEntry, uint64_t hash)
{
  // find open slot for value in transposition table:
  uint64_t iLock = hbin(hash);
  uint64_t key = h(hash);
  boost::unique_lock<spinlock> _lock(locks[iLock]);

#ifdef _HTCOLLECTSTATISTICS
  totalStores++;
#endif

  table_entry* worstEntry = &hashTable[key];
  // probe no more than the size of the bin
  for (size_t iProbe = 0; iProbe != binSize; ++iProbe)
  {
    table_entry* possible = &hashTable[key + iProbe];

    // is the entry not in use?
    if ( !possible->inUse() )
    {
      *possible = cEntry;

      // increment number of elements in ttable by 1
      numElements++;

#ifdef _HTCOLLECTSTATISTICS
      switch (cEntry.getStatus())
      {
      case NODET_CUTOFF:
      case NODET_CCUTOFF:
        numPartiallyEvaluatedNodes++;
        break;
      case NODET_FULLEVAL:
      case NODET_CFULLEVAL:
        numFullyEvaluatedNodes++;
        break;
      }
#endif
      return true;
    }
    // if the entry is in use, it's possible we might be probing for the same entry:
    else if ( *possible == cEntry )
    {
#ifdef _HTCOLLECTSTATISTICS
      unsigned int prevStatus = possible->getStatus();
#endif

      if (possible->update(cEntry))
      {
#ifdef _HTCOLLECTSTATISTICS
        // remove previous count
        switch (prevStatus)
        {
        case NODET_CUTOFF:
        case NODET_CCUTOFF:
          numPartiallyEvaluatedNodes--;
          break;
        case NODET_FULLEVAL:
        case NODET_CFULLEVAL:
          numFullyEvaluatedNodes--;
          break;
        }
        // add new count
        switch (cEntry.getStatus())
        {
        case NODET_CUTOFF:
        case NODET_CCUTOFF:
          numPartiallyEvaluatedNodes++;
          break;
        case NODET_FULLEVAL:
        case NODET_CFULLEVAL:
          numFullyEvaluatedNodes++;
          break;
        }
#endif
        return true;
      }
      return false;
    }
    // the entry was in use, determine if it is the worst entry in the bin
    else if ( *possible <= *worstEntry )
    {
      worstEntry = possible;
    }
    // else: entry in this bin is better than the current entry, do not replace
  } // endOf foreach probe up to maximum (elements)

  // if we reached the end of the bin before we reached a spot to place our entry:
  // replace worstEntry with possible
  assert(worstEntry != NULL);

#ifdef _HTCOLLECTSTATISTICS
  // remove previous count
  switch (worstEntry->getStatus())
  {
  case NODET_CUTOFF:
  case NODET_CCUTOFF:
    numPartiallyEvaluatedNodes--;
    break;
  case NODET_FULLEVAL:
  case NODET_CFULLEVAL:
    numFullyEvaluatedNodes--;
    break;
  }
  // add new count
  switch (cEntry.getStatus())
  {
  case NODET_CUTOFF:
  case NODET_CCUTOFF:
    numPartiallyEvaluatedNodes++;
    break;
  case NODET_FULLEVAL:
  case NODET_CFULLEVAL:
    numFullyEvaluatedNodes++;
    break;
  }
#endif

  *worstEntry = cEntry;

  return true;
}




table_entry transposition_table::findElement(const EnvironmentPossible& element)
{
  uint64_t iLock = hbin(element.getHash());
  boost::unique_lock<spinlock> _lock(locks[iLock]);
  // could the value at result be overwritten before it was copied successfully?
  table_entry* result = findElement_nonlocking(element);
  if (result == NULL)
  {
    return entry_deleted;
  }
  else
  {
    return *result;
  }
}





table_entry* transposition_table::findElement_nonlocking(
  const EnvironmentPossible& _element)
{
  assert(_element.getHash() != UINT64_MAX);
  uint64_t key = h(_element.getHash()); // generate array index
  table_entry* result = NULL;

  // search for hash:
  for (size_t iProbe = 0; iProbe != binSize; ++iProbe)
  {
    table_entry* possible = &hashTable[key + iProbe];

#ifdef _HTCOLLECTSTATISTICS
    totalProbes++;
#endif

    if ( !possible->inUse() ) 
    {
      // no element past this one, end search early
      return NULL;
    }
    else
    {
      if (*possible != _element)
      {
        // this probe did not yield a match, but there may be an element past this one in the current bin
        continue;
      }
      else // found match
      {
        result = possible;
        break;
      }
    }

  } // end of forEach probe up to maximum number of values in bin

#ifdef _HTCOLLECTSTATISTICS
  if (result != NULL)
  {
    totalProbeHits++;
  }
#endif

  // return pointer to resulting table_entry in hash table
  return result;
}





transposition_table::~transposition_table()
{
  // delete table
  hashTable.clear();
}





void transposition_table::clear()
{
  if (numElements == 0) { return; }

  // memset appears to be at least twice as fast than the intrinsic written solution that was previously here
  std::memset(&hashTable.front(), 0, sizeof(table_entry)*hashTable.size());

  numElements = 0;
#ifdef _HTCOLLECTSTATISTICS
  numFullyEvaluatedNodes = 0;
  numPartiallyEvaluatedNodes = 0;
#endif
}





size_t transposition_table::deleteOld()
{
  if (numElements == 0) { return 0; }

  size_t numDeleted = 0;
#ifdef _HTCOLLECTSTATISTICS
  uint64_t numFullDeleted = 0;
  uint64_t numPartialDeleted = 0;
#endif
  // pass 1: find all elements above numUsages usages and mark them as deleted
  for (size_t iElement = 0, iSize = maxElements; iElement != iSize; ++iElement)
  {
    table_entry& node = hashTable[iElement];

    if (!node.inUse()) { continue; }
    else
    {
#ifdef _HTCOLLECTSTATISTICS
      switch (node.getStatus())
      {
      case NODET_CUTOFF:
      case NODET_CCUTOFF:
        numPartialDeleted++;
        break;
      case NODET_FULLEVAL:
      case NODET_CFULLEVAL:
        numFullDeleted++;
        break;
      }
#endif
      numDeleted++;

      node.delEntry();
    }
  }

  // pass 2: assert that there are no elements that have been deleted between the beginning of a given bin array 
  //  and a given element
  for (size_t iBin = 0; iBin != numBins; ++iBin)
  {
    table_entry* currentBin = &hashTable[iBin * binSize];

    for (size_t iEntry = 0; iEntry != binSize; ++iEntry)
    {
      table_entry* nodeToMove = &currentBin[binSize - iEntry - 1];

      if ( ! nodeToMove->inUse() ) { continue; }

      for (size_t iMovedToEntry = iEntry + 1; iMovedToEntry != binSize; ++iMovedToEntry)
      {
        table_entry* nodeDestination = &currentBin[binSize - iMovedToEntry - 1];

        if ( !nodeDestination->inUse() ) 
        { 
          // there's a slot between the start of the bin and the current node in the bin, 
          // and we intend to fill it with nodeToMove
          *nodeDestination = *nodeToMove;
          nodeToMove->delEntry();
          break;
        }
      } // endOf foreach possible moved to slot
    } // endOf foreach node that might need to be moved
  } // endOf foreach bin

  assert(numElements >= numDeleted);
  numElements -= numDeleted;
#ifdef _HTCOLLECTSTATISTICS
  assert((numFullDeleted + numPartialDeleted) == numDeleted);
  numFullyEvaluatedNodes -= numFullDeleted;
  numPartiallyEvaluatedNodes -= numPartialDeleted;
#endif

  return numDeleted;
}





#ifdef _HTCOLLECTSTATISTICS
void transposition_table::resetStatistics()
{
  //boost::unique_lock<spinlock> _lock(locks[0]);

  totalProbes = 0;
  totalStores = 0;
  totalProbeHits = 0;
}
#endif





void transposition_table::resize(size_t _numBins, size_t _binSize)
{
  assert(false && "method not implemented yet!");
}