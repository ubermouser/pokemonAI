//#define PKAI_IMPORT
#include "../inc/orderHeuristic.h"

#define getPath(_cPokemon, _oPokemon) (_cPokemon*6 + _oPokemon)

orderHeuristic::orderHeuristic(bool randomize)
  : locks(),
  paths(),
  orders()
{
  if (randomize == true)
  {
    // foreach pokemon we need a path for:
    for (size_t iPath = 0; iPath < paths.size(); iPath++)
    {
      boost::array<path, AT_ITEM_USE+1>& cPath = paths[iPath];

      // assign each action a randomish value in order:
      
      for (size_t iAction = 0; iAction < cPath.size(); iAction++)
      {
        cPath[iAction].cutoffCount = rand() % cPath.size();
        cPath[iAction].useCount = cPath[iAction].cutoffCount;
      }
      
    }
  }

  sortInitial();
}





void orderHeuristic::reset(bool randomize)
{
  // lock the value while we reset:
  //boost::unique_lock<spinlock> _lock(lock);

  // foreach pokemon we need a path for:
  for (size_t iPath = 0; iPath < paths.size(); iPath++)
  {
    boost::array<path, AT_ITEM_USE+1>& cPath = paths[iPath];

    // assign each action in order:
    for (size_t iAction = 0; iAction < cPath.size(); iAction++)
    {
      if (randomize == true)
      {
        cPath[iAction].cutoffCount = rand() % cPath.size();
        cPath[iAction].useCount = cPath[iAction].cutoffCount;
      }
      else
      {
        cPath[iAction].cutoffCount = 0;
        cPath[iAction].useCount = 1;
      }
    }
  }

  sortInitial();
}





void orderHeuristic::incrementCutoff(uint8_t depth, size_t cPokemon, size_t oPokemon, uint8_t iAction)
{
  assert(cPokemon < 6);
  assert(oPokemon < 6);
  assert(iAction < AT_ITEM_USE + 1);

  size_t iPath = getPath(cPokemon, oPokemon);
  boost::array<path, AT_ITEM_USE+1>& cPath = paths[iPath];
  boost::array<uint8_t, AT_ITEM_USE+1>& cOrder = orders[iPath];

  path* currentPath = &cPath[iAction];

  // lock the value while we increment:
  boost::unique_lock<spinlock> _lock(locks[iPath]);

  // have we gone past the uint32_t size limit?
  assert((currentPath->cutoffCount + depth) > currentPath->cutoffCount);

  // increment that action:
  currentPath->cutoffCount += depth;

  // do not attempt to change place if this path is already in first place
  if (currentPath->place == 0) { return; }

  // test if action is greater than next action:
  path* nextBestPath = &cPath[ cOrder[currentPath->place - 1] ];

  while (*nextBestPath < *currentPath)
  {
    // swap the two!
    uint32_t tempPlace = currentPath->place;
    currentPath->place = nextBestPath->place;
    nextBestPath->place = tempPlace;

    cOrder[tempPlace] = cOrder[tempPlace - 1];
    cOrder[tempPlace - 1] = iAction;

    // do not attempt to change place if this path is already in first place
    if (currentPath->place == 0) { break; }
    nextBestPath = &cPath[ cOrder[currentPath->place - 1] ];
  }
}





void orderHeuristic::incrementUse(uint8_t depth, size_t cPokemon, size_t oPokemon, uint8_t iAction)
{
  assert(cPokemon < 6);
  assert(oPokemon < 6);
  assert(iAction < AT_ITEM_USE + 1);

  size_t iPath = getPath(cPokemon, oPokemon);
  boost::array<path, AT_ITEM_USE+1>& cPath = paths[iPath];
  boost::array<uint8_t, AT_ITEM_USE+1>& cOrder = orders[iPath];

  path* currentPath = &cPath[iAction];

  // lock the value while we increment:
  boost::unique_lock<spinlock> _lock(locks[iPath]);

  // have we gone past the uint32_t size limit?
  assert((currentPath->useCount + depth) > currentPath->useCount);

  // increment that action:
  currentPath->useCount += depth;

  // do not attempt to change place if this path is already in last place
  if (currentPath->place == AT_ITEM_USE) { return; }

  // test if action is greater than next action:
  path* nextWorstPath = &cPath[ cOrder[currentPath->place + 1] ];

  while (*currentPath < *nextWorstPath)
  {
    // swap the two!
    uint32_t tempPlace = currentPath->place;
    currentPath->place = nextWorstPath->place;
    nextWorstPath->place = tempPlace;

    cOrder[tempPlace] = cOrder[tempPlace + 1];
    cOrder[tempPlace + 1] = iAction;

    // do not attempt to change place if this path is already in last place
    if (currentPath->place == AT_ITEM_USE) { return; }
    nextWorstPath = &cPath[ cOrder[currentPath->place + 1] ];
  }
}





void orderHeuristic::seedOrdering(boost::array<uint8_t, AT_ITEM_USE+1>& ordering, size_t cPokemon, size_t oPokemon, int8_t killerMove)
{
  assert(cPokemon < 6);
  assert(oPokemon < 6);

  size_t iOrder = 0;
  if (killerMove >= 0)
  {
    assert((size_t)killerMove < ordering.size());

    // assert killer move first:
    ordering[0] = (uint8_t) killerMove;
    iOrder++;
  }

  size_t iPath = getPath(cPokemon, oPokemon);
  boost::array<uint8_t, AT_ITEM_USE+1>& cOrder = orders[iPath];

  // lock the value while we seed:
  boost::unique_lock<spinlock> _lock(locks[iPath]);

  // O(n)
  for (size_t iAction = 0 ; iAction < cOrder.size(); iAction++)
  {
    // don't add the killer move twice:
    if ( cOrder[iAction] == killerMove) { continue; }

    ordering[iOrder] = cOrder[iAction];
    iOrder++;
  }

  assert(isValidOrder(ordering));
}





void orderHeuristic::sortInitial()
{
  for (size_t iPokemon = 0, iSize = paths.size(); iPokemon != iSize; iPokemon++)
  {
    sortInitial_perPath(iPokemon);
  }
}





void orderHeuristic::sortInitial_perPath(size_t iPath)
{
  boost::array<path, AT_ITEM_USE+1>& cPath = paths[iPath];
  boost::array<uint8_t, AT_ITEM_USE+1>& cOrder = orders[iPath];

  boost::array<bool, AT_ITEM_USE+1> isChosen;
  isChosen.assign(false);

  // WARNING: O(N^2) algorithm!

  // for each index in the sorted list cOrder:
  for (size_t iAction = 0; iAction < cPath.size(); iAction++)
  {
    // determine if iBestAction is the current best action by searching through the entire array:
    size_t iBestAction = cPath.size();
    for (size_t ipAction = 0; ipAction < cPath.size(); ipAction++)
    {
      // if chosen already, don't consider it
      if (isChosen[ipAction] == true) { continue; }
      // if first item searched, by default consider it
      if (iBestAction == cPath.size()) { iBestAction = ipAction; }
      // if the current best action is worse than this possible best action, swap
      if (cPath[iBestAction] < cPath[ipAction]) { iBestAction = ipAction; }
    }

    // best possible action, given all actions at previous indecies have been evaluated
    cOrder[iAction] = (uint8_t) iBestAction;
    cPath[iBestAction].place = (uint8_t) iAction;
    isChosen[iBestAction] = true;
  }
}





bool orderHeuristic::isValidOrder(boost::array<uint8_t, AT_ITEM_USE+1>& ordering)
{
  boost::array<bool, AT_ITEM_USE+1> isChosen;
  isChosen.assign(false);
  unsigned int numChosen = 0;

  // for each index in the sorted list ordering:
  for (size_t iAction = 0; iAction < ordering.size(); iAction++)
  {
    if (isChosen[ordering[iAction]] == false) 
    { 
      isChosen[ordering[iAction]] = true; 
      numChosen++;
    }
    // this value was chosen previously!
    else { return false; }
  }

  if (numChosen == AT_ITEM_USE+1) { return true; }
  else { return false; }
}
