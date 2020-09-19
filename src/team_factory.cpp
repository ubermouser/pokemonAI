#include "../inc/team_factory.h"


void TeamFactory::mutate_single(TeamNonVolatile& team, size_t iTeammate, size_t numMutations) {
  PokemonNonVolatile& cPokemon = team.teammate(iTeammate);

  // if it's requested that we perform more than 8 unique mutations, just create a random teammate
  if (numMutations > 8) { cPokemon  = createRandom_single(cRankteam.team, iTeammate); return; }

  std::array<bool, 9> isMutated;
  isMutated.fill(false);

  for (size_t iMutation = 0; iMutation != numMutations; ++iMutation)
  {
    unsigned int mutationType = rand() % 27;
    // only select a mutation that has not been performed yet:
    while (isMutated[mutationType / 3])
    {
      mutationType = (mutationType + 3) % 27;
    };

    // don't perform this mutation again:
    isMutated[mutationType / 3] = true;

    switch(mutationType) {
    case 0: // change gender
    case 1:
    case 2:
      randomGender(cPokemon);
      break;
    case 3: // change ability
    case 4:
    case 5:
      randomAbility(cPokemon);
      break;
    case 6: // change held item:
    case 7:
    case 8:
      randomItem(cPokemon);
      break;
    case 9: // change EVs:
    case 10:
    case 11:
      randomEV(cPokemon);
      break;
    case 12: // change IVs:
    case 13:
    case 14:
      randomIV(cPokemon, (rand()%6) + 1);
      break;
    case 15: // change one or more moves:
    case 16:
    case 17:
      randomMove(cPokemon, (rand()%4) + 1);
      break;
    case 18: // change nature:
    case 19:
    case 20:
      randomNature(cPokemon);
      break;
    case 21: // change species:
    case 22:
    case 23:
      // TODO: probability of choosing a similar species instead of a random one
      randomSpecies(cRankteam.team, cPokemon, iTeammate);
      break;
    case 24: // change EVERYTHING:
    case 25:
    case 26:
      cPokemon = createRandom_single(cRankteam.team, iTeammate);
      return; // there's no point in performing any more random changes if we've changed EVERYTHING
    } // endOf mutation Switch
  } // endOf foreach mutation
} // endOf mutate_single


TeamNonVolatile TeamFactory::crossover(
  const TeamNonVolatile& parentA,
  const TeamNonVolatile& parentB)
{
  assert(parentA.team.getNumTeammates() == parentB.team.getNumTeammates());
  std::vector<const Ranked*> componentTeams;
  componentTeams.push_back(&parentA);
  componentTeams.push_back(&parentB);
  std::vector<fpType> numCTeam(2, 0.0);
  RankedTeam crossedTeam;
  size_t maxGeneration = std::max(parentA.generation, parentB.generation);
  size_t numTeammates = parentA.team.getNumTeammates();

  {
    TeamNonVolatile& cTeam = crossedTeam.team;
    // first element of each team to begin crossover loop with:
    size_t iParentA = rand() % numTeammates;
    size_t iParentB = rand() % numTeammates;

    for (size_t iCrossover = 0; iCrossover != numTeammates; ++iCrossover)
    {
      unsigned int crossoverType;
      if (numTeammates > 1) {crossoverType = rand() % 2;}
      else { crossoverType = 3; }
      switch(crossoverType)
      {
      case 0: // fully expressed parent A
        {
          // try adding parentA's pokemon first:
          const PokemonNonVolatile& candidate = parentA.team.teammate(iParentA);
          if (crossedTeam.team.isLegalAdd(candidate)) { cTeam.addPokemon(candidate); numCTeam[0]+= 1.0; break; }

          // try adding parentB's pokemon:
          const PokemonNonVolatile& backup = parentB.team.teammate(iParentB);
          if (crossedTeam.team.isLegalAdd(backup)) { cTeam.addPokemon(backup); numCTeam[1]+= 1.0; break; }

          // try adding a random pokemon: (guaranteed to be a legal add by createRandom_single)
          PokemonNonVolatile cTeammate = createRandom_single(crossedTeam.team);
          if (!cTeam.isLegalAdd(cTeammate)) { return createRandom(settings, numTeammates, 0); }
          cTeam.addPokemon(cTeammate);
          break;
        }
      case 1: // fully expressed parent B
        {
          // try adding parentB's pokemon first:
          const PokemonNonVolatile& candidate = parentB.team.teammate(iParentB);
          if (crossedTeam.team.isLegalAdd(candidate)) { cTeam.addPokemon(candidate); numCTeam[TEAM_B]+= 1.0; break; }

          // try adding parentA's pokemon:
          const PokemonNonVolatile& backup = parentA.team.teammate(iParentA);
          if (crossedTeam.team.isLegalAdd(backup)) { cTeam.addPokemon(backup); numCTeam[TEAM_A]+= 1.0; break; }

          // try adding a random pokemon: (guaranteed to be a legal add by createRandom_single)
          PokemonNonVolatile cTeammate = createRandom_single(crossedTeam.team);
          if (!cTeam.isLegalAdd(cTeammate)) { return createRandom(settings, numTeammates, 0); }
          cTeam.addPokemon(cTeammate);
          break;
        }
        break;
      case 2: // partially expressed A or B
      case 3:
        {
          // generate a crossover of parentA and parentB's pokemon:
          PokemonNonVolatile candidate = crossover_single(parentA.team.teammate(iParentA), parentB.team.teammate(iParentB));
          if (crossedTeam.team.isLegalAdd(candidate)) { cTeam.addPokemon(candidate); break; }
          // try adding a random pokemon: (guaranteed to be a legal add by createRandom_single)
          candidate = createRandom_single(crossedTeam.team);
          if (!cTeam.isLegalAdd(candidate)) { return createRandom(settings, numTeammates, 0); }
          cTeam.addPokemon(candidate);
          break;
        }
        break;
      }

      // increment parent indecies:
      iParentA = (iParentA + 1) % numTeammates;
      iParentB = (iParentB + 1) % numTeammates;
    } // endOf foreach crossover
  }

  // set generation of team:
  crossedTeam.generation = maxGeneration + 1;

  BOOST_FOREACH(fpType& cProportion, numCTeam)
  {
    cProportion /= ((fpType)numTeammates) * 1.5;
  }

  // determine team's skill from its component skills (crossovers are assumed to have default skill)
  crossedTeam.getSkill() = TrueSkill::teamRank(componentTeams.begin(), componentTeams.end(), numCTeam.begin(), 1.0, settings);
  //crossedTeam.getSkill().feather(settings);

  // reset the team's win record:
  //crossedTeam.resetRecord(); (performed implicitly by creation)

  // generate hash of the team (not components)
  crossedTeam.generateHash();

  // rename the team:
  crossedTeam.defineNames();

  // set that the team has not been saved:
  crossedTeam.stateSaved = false;

  return crossedTeam;
} // endOf crossover


PokemonNonVolatile TeamFactory::crossover_single(
  const PokemonNonVolatile& parentA,
  const PokemonNonVolatile& parentB)
{
  const PokemonNonVolatile& basePokemon = (((rand()%2)==TEAM_A)?parentA:parentB);
  const PokemonNonVolatile& otherPokemon = (&basePokemon==&parentA)?parentB:parentA;

  // basePokemon maintains species, ability, moveset
  PokemonNonVolatile crossedPokemon(basePokemon);

  // otherokemon maintains EV, IV, nature, gender, item
  for (size_t iIEV = 0; iIEV != 6; ++iIEV)
  {
    crossedPokemon.setEV(iIEV, otherPokemon.getEV(iIEV));
    crossedPokemon.setIV(iIEV, otherPokemon.getIV(iIEV));
  }

  // nature:
  crossedPokemon.setNature(otherPokemon.getNature());
  // item:
  if (otherPokemon.hasInitialItem())
  {
    crossedPokemon.setInitialItem(otherPokemon.getInitialItem());
  }
  else
  {
    crossedPokemon.setNoInitialItem();
  }
  // sex:
  crossedPokemon.setSex(otherPokemon.getSex());

  return crossedPokemon;
} // endOf crossover_single


TeamNonVolatile TeamFactory::createRandom(size_t numPokemon)
{
  assert(numPokemon >= 1 && numPokemon <= TeamNonVolatile::getMaxNumTeammates());
  TeamNonVolatile cTeam;

  for (size_t iTeammate = 0; iTeammate < numPokemon; ++iTeammate)
  {
    cTeam.addPokemon(createRandom_single(cTeam));
  }

  return cTeam;
}; // end of createRandom_team


PokemonNonVolatile TeamFactory::createRandom_single(const TeamNonVolatile& cTeam, size_t iReplace)
{
  PokemonNonVolatile cPokemon;

  // determine species:
  randomSpecies(cTeam, cPokemon, iReplace);

  // determine level:
  cPokemon.setLevel(100);

  // generate IVs:
  randomIV(cPokemon, 6);

  // generate EVs:
  randomEV(cPokemon);

  // determine nature, maximizing IVs and EVs:
  randomNature(cPokemon);

  // determine gender based on species:
  randomGender(cPokemon);

  // determine ability based on species:
  randomAbility(cPokemon);

  // determine held item:
  randomItem(cPokemon);

  // determine moves based on species:
  randomMove(cPokemon, 4);

  return cPokemon;
} // endOf createRandom_single


void TeamFactory::randomSpecies(const TeamNonVolatile& cTeam, PokemonNonVolatile& cPokemon, size_t iReplace)
{
  std::vector<const PokemonBase*> pokemons = pkdex->getPokemon().toVector();
  bool revalidate = cPokemon.pokemonExists();
  bool isSuccessful = false;
  size_t iSpecies = (rand() % pokemons.size()) - 1;
  // find a pokemon with an existing movelist (non orphan)
  for (size_t iNSpecies = 0; iNSpecies != pokemons.size(); ++iNSpecies)
  {
    iSpecies = (iSpecies + 1) % pokemons.size();
    const PokemonBase& candidateBase = *pokemons[iSpecies];

    // don't include a species already on the team:
    if ((iReplace == SIZE_MAX) && !cTeam.isLegalAdd(candidateBase)) { continue; }
    else if (!cTeam.isLegalSet(iReplace, candidateBase)) { continue; }
    // don't include any pokemon species with lost children: (no moves, no ability, no types)
    if (candidateBase.lostChild_ == true) { continue; }

    isSuccessful = true;
    cPokemon.setBase(candidateBase);
    break;
  }
  if (!isSuccessful)
  {
    // something horrible happened, do not attempt to randomize the species. No need to revalidate if this occurs
    assert(false && "Failed to generate random species!");
    return;
  }

  if (revalidate) {
    // remove invalid moves:
    size_t numInvalidMoves = 0;
    for (size_t iNMove = 0, iSize = cPokemon.getNumMoves(); iNMove != iSize; ++iNMove)
    {
      // increment in reverse order, since a delete will remove the last element from the move array
      size_t iMove = iSize - iNMove - 1;

      if (!cPokemon.isLegalSet(iMove + AT_MOVE_0, cPokemon.getMove(iMove + AT_MOVE_0)))
      {
        cPokemon.removeMove(iMove + AT_MOVE_0);
        numInvalidMoves++;
      }
    }
    // and replace the invalid moves with new, valid ones:
    if (numInvalidMoves > 0)
    {
      randomMove(cPokemon, numInvalidMoves);
    }

    // determine if ability is invalid:
    if (cPokemon.abilityExists())
    {
      const PokemonBase& cBase = cPokemon.getBase();
      bool isMatched = cBase.abilities_.count(&cPokemon.getAbility());
      if (!isMatched)
      {
        // and update with a new ability if it is
        randomAbility(cPokemon);
      }
    }

    assert(cPokemon.getNumMoves() > 0);
  } // endOf revalidation
} // endOf randomSpecies


void TeamFactory::randomAbility(PokemonNonVolatile& cPokemon)
{
  const PokemonBase& cBase = cPokemon.getBase();

  std::vector<const Ability*> abilities{begin(cBase.getAbilities()), end(cBase.getAbilities())};
  size_t iAbility = rand() % abilities.size();
  for (size_t iNAbility = 0; iNAbility != abilities.size(); ++iNAbility)
  {
    if (abilities[iAbility]->isImplemented()) { break; }

    iAbility = (iAbility + 1) % abilities.size();
  }

  // if the ability does not have a script implemented:
  if (!abilities[iAbility]->isImplemented())
  {
    cPokemon.setNoAbility();
  }
  else
  {
    cPokemon.setAbility(*abilities[iAbility]);
  }
} // endOf randomAbility


void TeamFactory::randomNature(PokemonNonVolatile& cPokemon)
{
  std::vector<const Nature*> natures = pkdex->getNatures().toVector();
  size_t iNature = rand() % natures.size();
  cPokemon.setNature(*natures[iNature]);
}


void TeamFactory::randomItem(PokemonNonVolatile& cPokemon)
{
  std::vector<const Item*> items = pkdex->getItems().toVector();
  size_t iNSize = (items.size() + 1);
  size_t iItem = (rand() % iNSize) - 1;
  for (size_t iNItem = 0; iNItem != iNSize; ++iNItem)
  {
    // in this case, we choose no item:
    if (iItem == SIZE_MAX) { break; }

    if (items[iItem]->isImplemented()) { break; }

    iItem = ((iItem + 2) % iNSize) - 1;
  }

  // if the item does not have a script implemented or we explicitly chose no item:
  if ((iItem == SIZE_MAX) || (!items[iItem]->isImplemented()))
  {
    cPokemon.setNoInitialItem();
  }
  else
  {
    cPokemon.setInitialItem(*items[iItem]);
  }
} // endOf randomItem


void TeamFactory::randomIV(PokemonNonVolatile& cPokemon, size_t numIVs)
{
  std::array<bool, 6> isValid;
  isValid.fill(true);
  bool isSuccessful;
  for (size_t _iIV = 0; _iIV != std::max(numIVs, (size_t)6); ++_iIV)
  {
    isSuccessful = false;
    size_t iIV;
    for (size_t numTries = 0; (numTries != MAXTRIES) && (!isSuccessful); ++numTries)
    {
      iIV = rand() % 6;

      // has this IV been randomized yet?
      if (!isValid[iIV]) { continue; }

      isSuccessful = true;
    }

    // if we were not successful in finding a move to go into this slot, just pick the first one we can find:
    if (!isSuccessful)
    {
      for (size_t iPIV = 0; iPIV != 6; ++iPIV)
      {
        if (isValid[iPIV] == true)
        {
          iIV = iPIV;
          isSuccessful = true;
          break;
        }
      }
      // if we were unsuccessful a second time, this means we've randomized all possible IVs
      if (!isSuccessful)
      {
        return;
      }
    }

    // don't randomize the same IV twice
    isValid[iIV] = false;
    uint32_t IVStatus = rand() % 7;

    switch(IVStatus)
    {
    case 0:
      cPokemon.setIV(iIV, 0);
      break;
    case 1:
    case 2:
      cPokemon.setIV(iIV, 30);
      break;
    case 3:
    case 4:
    case 5:
    case 6:
    default:
      cPokemon.setIV(iIV, 31);
      break;
    }

  }
} // endOf randomIV


void TeamFactory::randomEV(PokemonNonVolatile& cPokemon)
{
  std::array<unsigned int, 6> tempEV;
  uint32_t evAccumulator;
  bool isSuccessful = false;
  for (size_t numTries = 0; (numTries != MAXTRIES) && (!isSuccessful); ++numTries)
  {
    evAccumulator = 0;
    size_t iEV = rand() % 6;
    for (size_t iValue = 0; iValue != 6; ++iValue)
    {


      // only multiples of 4
      uint32_t EV = std::min((unsigned)(rand() % 64), (unsigned)((MAXEFFORTVALUE - evAccumulator)/4));
      EV *= 4;

      evAccumulator += EV;
      tempEV[iEV] = EV;

      iEV = (iEV + 1) % 6;
    }

    isSuccessful = (evAccumulator >= MAXEFFORTVALUE - 4) && (evAccumulator <= MAXEFFORTVALUE);
  }

  // if we were not successful creating a viable EV set, use a default:
  if (!isSuccessful)
  {
    for (size_t iEV = 0; iEV < 6; iEV++)
    {
      tempEV[iEV] = std::min(MAXEFFORTVALUE / 6, 255);
    }
  }

  // writeOut evs:
  for (size_t iEV = 0; iEV < 6; iEV++)
  {
    cPokemon.setEV(iEV, tempEV[iEV]);
  }
} // endOf randomEV


void TeamFactory::randomMove(PokemonNonVolatile& cPokemon, size_t numMoves)
{
  const std::vector<const Move*>& cMovelist{
      begin(cPokemon.getBase().moves_), end(cPokemon.getBase().moves_)};
  std::vector<bool> isValid(cMovelist.size(), true);
  size_t validMoves = cMovelist.size();

  // remove duplicate moves from the valid move array:
  for (size_t iMove = 0; iMove != cPokemon.getNumMoves(); ++iMove)
  {
    const Move* base = &cPokemon.getMove_base(iMove + AT_MOVE_0);

    for (size_t iValidMove = 0, iValidSize = cMovelist.size(); iValidMove != iValidSize; ++iValidMove)
    {
      if (base == cMovelist[iValidMove]) { isValid[iValidMove] = false; validMoves--; break; }
    }
  }

  // add numMoves moves to the move array:
  for (size_t iAction = 0, iSize = std::min(numMoves, cMovelist.size()); iAction < std::min(iSize, validMoves); ++iAction)
  {
    // the index of the move we intend to add:
    size_t iMove;
    // the slot we intend to put it in:
    size_t iSlot = rand() % cPokemon.getMaxNumMoves();
    bool isSuccessful = false;
    for (size_t numTries = 0; (numTries != MAXTRIES) && (!isSuccessful); ++numTries)
    {
      iMove = rand() % cMovelist.size();

      const Move& possibleMove = *cMovelist.at(iMove);

      // has this move been used before?
      if (!isValid.at(iMove))
      {
        continue;
      }

      // is this move wholly implemented via the engine, or does it have scripts that are fully implemented?
      if (!possibleMove.isImplemented())
      {
        isValid.at(iMove) = false;
        validMoves--;
        continue;
      }

      isSuccessful = true;
    }

    // if we were not successful in finding a move to go into this slot, just forget about it
    if (!isSuccessful) { continue; }

    isValid.at(iMove) = false;
    validMoves--;
    MoveNonVolatile cMove(*cMovelist.at(iMove));

    if ((cPokemon.getNumMoves() < numMoves) || (iSlot >= cPokemon.getNumMoves()))
    {
      cPokemon.addMove(cMove);
    }
    else
    {
      cPokemon.setMove(iSlot, cMove);
    }
  } //endOf iAction

  assert(cPokemon.getNumMoves() > 0);
} // endOf randomMove


void TeamFactory::randomGender(PokemonNonVolatile& cPokemon)
{
  unsigned int gender = rand() % 3;
  // TODO: allowed genders based on species
  cPokemon.setSex(gender);
}
