//#define PKAI_STATIC
#include "../inc/pokedex.h"
//#undef PKAI_STATIC

#include <stdint.h>
#include <algorithm>
#include <vector>
#include <iostream>
#include <sstream>
#include <iomanip>
#include <string>
#include <boost/foreach.hpp>

#include "../inc/game.h"
#include "../inc/genetic.h"
#include "../inc/roulette.h"
#include "../inc/init_toolbox.h"

#include "../inc/ranked_team.h"

const std::string ranked_team::header = "PKAIR1";

#define DELETECOMPONENTTEAMS(componentTeams, createdTeams) \
for (size_t iComponentTeam = 0; iComponentTeam != componentTeams.size(); ++iComponentTeam) \
{ \
  const ranked_team* cComponentTeam = (ranked_team*)componentTeams[iComponentTeam]; \
  if (createdTeams[iComponentTeam] == true) { delete cComponentTeam; } \
}





ranked_team::ranked_team(const team_nonvolatile& cTeam, size_t _generation, const trueSkillSettings& cSettings)
  : ranked(_generation, cSettings),
  team(cTeam),
  hash(),
  rankPoints(),
  numPlies(),
  numMoves()
{
  getSkill() = trueSkill(1, cSettings);
  hash.assign(ranked::defaultHash);
  numPlies.assign(0);
  rankPoints.assign(0);
  for (size_t iTeammate = 0; iTeammate != team.getMaxNumTeammates(); ++iTeammate)
  {
    numMoves[iTeammate].assign(0);
  }
};





ranked_team::ranked_team(const ranked_team& other)
  : ranked(other),
  team(other.team),
  hash(other.hash),
  rankPoints(other.rankPoints),
  numPlies(other.numPlies),
  numMoves(other.numMoves)
{
};





std::ostream& operator <<(std::ostream& os, const ranked_team& tR)
{
  os << 
    "" << std::setw(20) << tR.team.getName().substr(0,20) << " ";

  // print a few characters from each teammate:
  size_t teammateStringSize = 0;
  size_t sizeAccumulator = 0;
  for (size_t iTeammate = 0; iTeammate != tR.team.getNumTeammates(); ++iTeammate)
  {
    teammateStringSize = (24 - sizeAccumulator) / (tR.team.getNumTeammates() - iTeammate);

    const std::string& baseName = tR.team.teammate(iTeammate).getBase().getName();
    os << baseName.substr(0, teammateStringSize);

    sizeAccumulator += std::min(baseName.size(), teammateStringSize);
  }
  while (sizeAccumulator < 24) { os << " "; sizeAccumulator++; };

  size_t prevPrecision = os.precision();
  os.precision(6);
  os <<
    " g= " << std::setw(3) << std::right << tR.getGeneration() <<
    " m= " << std::setw(7) << tR.getSkill().getMean() <<
    " s= " << std::setw(7) << tR.getSkill().getStdDev() <<
    " w= " << std::setw(7) << std::left << tR.getNumWins() << 
    " / " << std::setw(7) << std::right << (tR.getNumGamesPlayed());
  os.precision(prevPrecision);
  
  return os;
}





void ranked_team::output(std::ostream& oFile, bool printHeader) const
{
  // header:
  if (printHeader)
  {
    oFile << header << "\n";
  };

  // team hash:
  oFile << std::hex << getHash() << std::dec << "\n";
  for (size_t iTeammate = 0; iTeammate != team.getNumTeammates(); ++iTeammate)
  {
    // pokemon hashes:
    oFile << std::hex << getTeammateHash(iTeammate) << std::dec << "\t";
  }
  oFile << "\n";

  // rankpoints:
  for (size_t iTeammate = 0; iTeammate != 6; ++iTeammate)
  {
    oFile << getNumRankPoints(iTeammate) << "\t";
  }
  oFile << "\n";

  // number of plies:
  for (size_t iTeammate = 0; iTeammate != 6; ++iTeammate)
  {
    oFile << getTeammatePlies(iTeammate) << "\t";
  }
  oFile << "\n";

  // number of moves per pokemon:
  for (size_t iTeammate = 0; iTeammate != 6; ++iTeammate)
  {
    for (size_t iMove = 0; iMove != 5; ++iMove)
    {
      oFile << getNumMovesUsed(iTeammate, iMove) << "\t";
    }
  }
  oFile << "\n";

  // ranked elements, trueskill:
  ranked::output(oFile);

  // the rest of this section is exactly like a normal output team method, except with a different header
  team.output(oFile);
} // endOf outputRankedTeam





bool ranked_team::input(const std::vector<std::string>& lines, size_t& iLine)
{
  /*
   * Header data:
   * PKAIR<VERSION
   * <team hash>\n
   * <pokemon 0 hash> <pokemon 1 hash> ...\n
   * <numPlies 1> <numPlies 1> <numPlies 2> ..\n
   * <move 0.0> <move 0.1> <move 0.2> ...\n
   * <numWins> <numLosses> <numTies> <numDraws>\n
   * STANDARD TEAM REMAINS
   */

  // are the enough lines in the input stream:
  if ((lines.size() - iLine) < 6U)
  {
    std::cerr << "ERR " << __FILE__ << "." << __LINE__ << 
      ": unexpected end of input stream at line " << iLine << "!\n";
    return false; 
  }

  // compare ranked_team header:
  if (lines.at(iLine).compare(0, header.size(), header) != 0)
  {
    std::cerr << "ERR " << __FILE__ << "." << __LINE__ << 
      ": ranked stream has header of type \"" << lines.at(iLine).substr(0, header.size()) << 
      "\" (needs to be \"" << header << 
      ") and is incompatible with this program!\n";

    return false;
  }

  // input team hash:
  iLine++;
  {
    std::string token(lines.at(iLine));
    std::istringstream tokenStream(token, std::istringstream::in);

    if (!(tokenStream >> std::hex >> hash[0])) { INI::incorrectArgs("ranked_team hash[0]", iLine, 0); return false; }
  }

  // input pokemon hashes:
  iLine++;
  {
    std::vector<std::string> tokens = INI::tokenize(lines.at(iLine), "\t");
    if (!INI::checkRangeB(tokens.size(), (size_t)1U, (size_t)6U)) { return false; }
    for (size_t iHash = 0; iHash != tokens.size(); ++iHash)
    {
      std::istringstream tokenStream(tokens.at(iHash));
      if (!(tokenStream >> std::hex >> hash[iHash + 1])) 
      { 
        INI::incorrectArgs("ranked_team hash", iLine, iHash); 
        return false; 
      }
    }
  }

  // input rankPoints:
  iLine++;
  {
    std::vector<std::string> tokens = INI::tokenize(lines.at(iLine), "\t");
    if (!INI::checkRangeB(tokens.size(), (size_t)6U, (size_t)6U)) { return false; }
    for (size_t iTeammate = 0; iTeammate != tokens.size(); ++iTeammate)
    {
      if (!INI::setArgAndPrintError("ranked_team rankPoints", tokens.at(iTeammate), rankPoints[iTeammate], iLine, iTeammate)) 
      { 
        return false; 
      }
    }
  }

  // input numPlies:
  iLine++;
  {
    std::vector<std::string> tokens = INI::tokenize(lines.at(iLine), "\t");
    if (!INI::checkRangeB(tokens.size(), (size_t)6U, (size_t)6U)) { return false; }
    for (size_t iTeammate = 0; iTeammate != tokens.size(); ++iTeammate)
    {
      if (!INI::setArgAndPrintError("ranked_team numPlies", tokens.at(iTeammate), numPlies[iTeammate], iLine, iTeammate)) 
      { 
        return false; 
      }
    }
  }

  // input numMoves:
  iLine++;
  {
    std::vector<std::string> tokens = INI::tokenize(lines.at(iLine), "\t");
    if (!INI::checkRangeB(tokens.size(), (size_t)30U, (size_t)30U)) { return false; }
    for (size_t iTeammate = 0; iTeammate != 6; ++iTeammate)
    {
      for (size_t iMove = 0; iMove != 5; ++iMove)
      {
        if (!INI::setArgAndPrintError("ranked_team numMoves", tokens.at(iTeammate*4 + iMove), numMoves[iTeammate][iMove], iLine, iTeammate*4 + iMove)) { return false; }
      }
    }
  }

  iLine++;

  // input ranked object:
  if (!ranked::input(lines, iLine)) { return false; }

  // input actual team:
  if (!team.input(lines, iLine)) { return false; }

  return true;
} // endOf inputTeam (ranked)





void ranked_team::generateHash(bool generateSubHashes)
{
  if (generateSubHashes)
  {
    // generate hashes for each of the pokemon:
    for (size_t iTeammate = 0; iTeammate < team.getNumTeammates(); ++iTeammate)
    {
      hash[iTeammate + 1] = team.teammate(iTeammate).hash();
    }
  }

  hash[0] = team.hash();
};





void ranked_team::defineNames()
{
  size_t numPokemon = team.getNumTeammates();
  // set name of created team based on hash:
  {
    std::ostringstream tName(std::ostringstream::out);
    tName << "_" << numPokemon << "-x" << std::setfill('0') << std::setw(16) << std::hex << getHash();
    team.setName(tName.str());
  }

  // set names of created pokemon based on hashes:
  for (size_t iTeammate = 0; iTeammate < numPokemon; ++iTeammate)
  {
    pokemon_nonvolatile& cPokemon = team.teammate(iTeammate);

    std::ostringstream tName(std::ostringstream::out);
    tName << "" << iTeammate << numPokemon << "-x" << std::setfill('0') << std::setw(16) << std::hex << getTeammateHash(iTeammate);
    cPokemon.setName(tName.str());
  }
} // endOf defineNames





size_t ranked_team::update(const game& cGame, const trueSkillTeam& cTeam, size_t iTeam)
{
  size_t numUpdates = 0;
  const std::vector<gameResult>& gameResults = cGame.getGameResults();

  // update ranked vars:
  ranked::update(cGame, cTeam, iTeam);
  BOOST_FOREACH(ranked* componentTeam, cTeam.subTeams)
  {
    componentTeam->update(cGame, cTeam, iTeam);
  }

  // update ranked_team specific vars:
  BOOST_FOREACH(const gameResult& cGameResult, gameResults)
  {
    const boost::array<boost::array<uint32_t, 5>, 6>& cTeamMoves = cGameResult.moveUse[iTeam];
    const boost::array<uint32_t, 6>& cTeamRanks = cGameResult.ranking[iTeam];
    const boost::array<uint32_t, 6>& cTeamPlies = cGameResult.participation[iTeam];
    for (size_t iTeammate = 0; iTeammate != team.getNumTeammates(); ++iTeammate)
    {
      const boost::array<uint32_t, 5>& cTeammateMoves = cTeamMoves[iTeammate];

      // main team:
      for (size_t iMove = 0, iMoveSize = team.teammate(iTeammate).getNumMoves(); iMove != iMoveSize; ++iMove)
      {
        // add number of a particular action this pokemon used during the match to total
        numMoves[iTeammate][iMove] += cTeammateMoves[iMove];
          
      }
      // add number of rounds teammate was in the game to total:
      numPlies[iTeammate] += cTeamPlies[iTeammate];
      // add rankPoints:
      rankPoints[iTeammate] += 
        team.getNumTeammates() - cTeamRanks[iTeammate];
    }
    numUpdates++;

    // subteams:
    size_t iComponent = 0;
    BOOST_FOREACH(ranked_team* componentTeam, cTeam.subTeams)
    {
      const boost::array<size_t, 6>& correspondence = cTeam.correspondencies[iComponent];

      for (size_t iOTeammate = 0; iOTeammate != componentTeam->team.getNumTeammates(); ++iOTeammate)
      {
        assert(correspondence[iOTeammate] != SIZE_MAX);

        const boost::array<uint32_t, 5>& cTeammateMoves = cTeamMoves[correspondence[iOTeammate]];
          
        // increment plies:
        componentTeam->numPlies[iOTeammate] += cTeamPlies[correspondence[iOTeammate]];
        // increment rankPoints: (weighted by total team's numTeammates)
        componentTeam->rankPoints[iOTeammate] += 
          team.getNumTeammates() - cTeamRanks[correspondence[iOTeammate]];

        // increment moves:
        for (size_t iMove = 0, iMoveSize = componentTeam->team.teammate(iOTeammate).getNumMoves(); iMove != iMoveSize; ++iMove)
        {
          // add number of a particular action this pokemon used during the match to total
          componentTeam->numMoves[iOTeammate][iMove] += cTeammateMoves[iMove];
          
        } // endOf foreach move of subteammate

      } // endOf foreach teammate of subteam
      numUpdates++;
    } //endOf foreach component team 
  } // endof foreach game

  return numUpdates;
} //endOf update





ranked_team ranked_team::selectRandom(
  const trueSkillSettings& settings,
  const boost::array<std::vector<ranked_team>, 6>& league, 
  size_t numPokemon)
{
  std::vector<const ranked*> componentTeams;
  std::vector<bool> createdTeams;
  std::vector<fpType> numCTeam;
  ranked_team cRankTeam;
  team_nonvolatile& cTeam = cRankTeam.team;
  size_t maxGeneration = 0;

  if (numPokemon > 1)
  {
    size_t numPokemonLeft = numPokemon;
    bool isSuccessful = false;
    for (size_t numTries = 0; (numTries != MAXTRIES) && (!isSuccessful); ++numTries)
    {
      // select a random team either via roulette or completely randomly:
      boost::array<size_t, 2> iRandomTeam = selectRandom_single(cRankTeam, league, std::min(numPokemonLeft, numPokemon - 1));

      // if we were unable to generate a team:
      const ranked_team* targetTeam = (iRandomTeam[0] == SIZE_MAX || iRandomTeam[1] == SIZE_MAX)
        // generate a random subsection of the team that isn't in the league:
        ? new ranked_team(createRandom(settings, numPokemonLeft, 0))
        // or select a subsection from a smaller league
        : &league[iRandomTeam[0]][iRandomTeam[1]];

      // push back a new component team
      componentTeams.push_back(targetTeam);
      numCTeam.push_back(0);

      // add collected pokemon to cRankTeam
      for (size_t iTeammate = 0, iSize = targetTeam->team.getNumTeammates(); iTeammate != iSize; ++iTeammate)
      {
        const pokemon_nonvolatile& cTeammate = targetTeam->team.teammate(iTeammate);

        // determine if pokemon is legal add: (selectRandom_single should guarantee this)
        if (!cTeam.isLegalAdd(cTeammate)) { continue; }

        cTeam.addPokemon(cTeammate);

        // add one pokemon from new team
        numCTeam.back() += 1.0;
        numPokemonLeft--;
      }
      //numCTeam.back() /= 2.0;

      // TODO: remove early if we didn't use any teammates from this team?
      if (iRandomTeam[0] == SIZE_MAX || iRandomTeam[1] == SIZE_MAX) { createdTeams.push_back(true); }
      else { createdTeams.push_back(false); }

      // determine if maximum generation is greater than iMaxGeneration
      if ((targetTeam->getGeneration() > maxGeneration) && (numCTeam.back() > 0))
      {
        maxGeneration = targetTeam->getGeneration();
      }

      isSuccessful = (numPokemonLeft == 0);
    } // endOf foreach add try

    // if we could not generate a component team, generate a random team:
    if (!isSuccessful)
    {
      // destroy teams we had to generate to create this team:
      DELETECOMPONENTTEAMS(componentTeams, createdTeams);

      return createRandom(settings, numPokemon, 0);
    }
  } // endOf numPokemon > 1
  else
  {
    return createRandom(settings, numPokemon, 0);
  }

  BOOST_FOREACH(fpType& cProportion, numCTeam)
  {
    cProportion /= ((fpType)numPokemon) * 1.5;
  }

  // determine team's skill from its component skills
  cRankTeam.getSkill() = trueSkill::teamRank(componentTeams.begin(), componentTeams.end(), numCTeam.begin(), 1.0, settings);
  //cRankTeam.getSkill().feather(settings);

  // set generation of team:
  cRankTeam.generation = maxGeneration + 1;

  // reset the team's win record:
  //cRankTeam.resetRecord(); (performed implicitly by creation)

  // generate hash of the team
  cRankTeam.generateHash();

  // rename the team:
  cRankTeam.defineNames();

  cRankTeam.stateSaved = false;

  // destroy teams we had to generate to create this team:
  DELETECOMPONENTTEAMS(componentTeams, createdTeams);

  return cRankTeam;
} // endOf selectRandom





boost::array<size_t, 2> ranked_team::selectRandom_single(
  const ranked_team& existing,
  const boost::array<std::vector<ranked_team>, 6>& league,
  size_t numPokemon,
  bool allowLess)
{
  bool isSuccessful = false;
  size_t iLeague, iTeam = SIZE_MAX;
  // if 1 pokemon, select only from league 0. 
  // if multiple pokemon and allowed to choose less than that multiple, rand
  // if multiple but not allowed to choose less, numPokemon
  iLeague = (allowLess)?(rand()%numPokemon):(numPokemon - 1);
  for (size_t iNLeague = 0; iNLeague != league.size(); ++iNLeague)
  {
    const std::vector<ranked_team>& cLeague = league[iLeague];

    // no possible matches
    if (!allowLess && cLeague.empty()) { break; }
    // are there any teams in this league at all?
    else if (cLeague.empty()) { iLeague = (iLeague + 1) % numPokemon; continue; }

    // select teammate, weighted by mean ability.
    // This method also determines if the team is a legal add
    iTeam = roulette<ranked_team, sortByMean_noDuplicates>::select(cLeague, sortByMean_noDuplicates(existing));

    // no possible matches
    if (!allowLess && iTeam == SIZE_MAX) { break; }
    // or a match could not be found
    else if (iTeam == SIZE_MAX) { iLeague = (iLeague + 1) % numPokemon; continue; }

    isSuccessful = true;
    break;
  }

  // if we were unable to select a team:
  if (!isSuccessful)
  {
    // failed result:
    boost::array<size_t, 2> failedResult = {{ SIZE_MAX, SIZE_MAX }};
    return failedResult;
  }

  boost::array<size_t, 2> successfulResult = {{ iLeague, iTeam }};
  return successfulResult;
} // endOf selectRandom_single





ranked_team ranked_team::createRandom(
  const trueSkillSettings& settings,
  size_t numPokemon, 
  size_t _generation)
{
  // sets team, hash, trueskill, NOT hash or name
  ranked_team cRankteam(createRandom(numPokemon), _generation, settings);

  // hash created pokemon:
  cRankteam.generateHash();

  cRankteam.defineNames();

  cRankteam.stateSaved = false;

  return cRankteam;
} //endOf createRandom 

team_nonvolatile ranked_team::createRandom(size_t numPokemon)
{
  assert(numPokemon >= 1 && numPokemon <= team_nonvolatile::getMaxNumTeammates());
  team_nonvolatile cTeam;

  for (size_t iTeammate = 0; iTeammate < numPokemon; ++iTeammate)
  {
    cTeam.addPokemon(createRandom_single(cTeam));
  }

  return cTeam;
}; // end of createRandom_team





ranked_team ranked_team::mutate(
  const trueSkillSettings& settings,
  const boost::array<std::vector<ranked_team>, 6>& league, 
  const ranked_team& parent,
  size_t numMutations)
{
  ranked_team mutatedTeam(parent);

  size_t numTeammates = mutatedTeam.team.getNumTeammates();
  size_t maxGeneration = mutatedTeam.getGeneration();

  // an array of which pokemon have been seeded from which team. Used for averaging trueskill scores
  std::vector<const ranked*> componentTeams;
  componentTeams.push_back(&parent);
  std::vector<fpType> numCTeam(1, (fpType)numTeammates);

  // mutate one of the pokemon on the team in some way, or
  // flip which pokemon is the lead pokemon,
  // or steal a subset from another team:
  unsigned int mutationType = (rand() % 4) + (numTeammates==1?0:27);
  switch(mutationType)
  {
  case 30: // swap a subset of the team for another team's portion in the tier: (cannot be chosen if numTeammates == 1)
  case 29:
  case 28:
    {
      // select a random team via roulette:
      boost::array<size_t, 2> iRandomTeam = selectRandom_single(mutatedTeam, league, (numTeammates - 1), true);

      // if we were unable to generate a team:
      const ranked_team targetTeam = (iRandomTeam[0] == SIZE_MAX || iRandomTeam[1] == SIZE_MAX)
        // generate a random subsection of the team
        ? createRandom(settings, (rand() % (numTeammates - 1)) + 1, 0)
        // or select a subsection from a smaller league
        : league[iRandomTeam[0]][iRandomTeam[1]];

      // push back a new component team
      componentTeams.push_back(&targetTeam);
      numCTeam.push_back(0.0);

      // add elements from targetTeam to mutatedTeam, but don't add an element over an already swapped teammate:
      std::vector<bool> isSwapped(numTeammates, false);
      for (size_t iOTeammate = 0; iOTeammate != targetTeam.team.getNumTeammates(); ++iOTeammate)
      {
        // find an acceptable target swap index:
        size_t iTarget = rand() % numTeammates;
        while (isSwapped[iTarget])
        {
          iTarget = (iTarget + 1) % numTeammates;
        };

        const pokemon_nonvolatile& swappedTeammate = targetTeam.team.teammate(iOTeammate);

        // determine if pokemon is legal add: (selectRandom_single should guarantee this)
        if (!mutatedTeam.team.isLegalAdd(swappedTeammate)) { continue; }
        
        // add pokemon if legal:
        mutatedTeam.team.setPokemon(iTarget, swappedTeammate);

        // remove 1 pokemon from current team, add one to new team
        numCTeam.front() -= 1.0;
        numCTeam.back() += 1.0;

        // set that the pokemon has been swapped (so as not to swap it again):
        isSwapped[iTarget] = true;
      }

      //numCTeam.back() /= (fpType)numTeammates;

      // determine if maximum generation is greater than iMaxGeneration
      if ((targetTeam.getGeneration() > maxGeneration) && (mostlyGT(numCTeam.back(), 0.0)))
      {
        maxGeneration = targetTeam.getGeneration();
      }

      break;
    }
  case 27: // set one of the existing pokemon as a new lead: (cannot be chosen if numTeammates == 1)
    {
      size_t iSwap = (rand() % (numTeammates - 1)) + 1; 
      
      mutatedTeam.team.setLeadPokemon(iSwap);
      //numCTeam.front() -= 1.0; // feather by 1
      break;
    }
  default: // mutation case for singles tier
    {
      size_t iMutate = rand() % numTeammates;
      size_t numMutations = (size_t) sqrt((fpType)(rand()%64)) + 1; // from 1 to 8 mutations
      mutate_single(mutatedTeam, iMutate, numMutations);
      numCTeam.front() -= 1.0; // feather by 1 (or feather the whole damn thing)
      break;
    }
  }

  BOOST_FOREACH(fpType& cProportion, numCTeam)
  {
    cProportion /= ((fpType)numTeammates) * 1.5;
  }

  // feather (not reset) the team's rank:
  mutatedTeam.getSkill() = trueSkill::teamRank(componentTeams.begin(), componentTeams.end(), numCTeam.begin(), 1.0, settings);
  //mutatedTeam.getSkill().feather(settings);

  // add one to the team's generation:
  mutatedTeam.generation++;

  // reset the team's win record:
  mutatedTeam.resetRecord();

  // rehash the team:
  mutatedTeam.generateHash();

  // rename the team:
  mutatedTeam.defineNames();

  // set that the team has not been saved:
  mutatedTeam.stateSaved = false;

  return mutatedTeam;
} // endOf mutate





ranked_team ranked_team::crossover(
  const trueSkillSettings& settings,
  const ranked_team& parentA, 
  const ranked_team& parentB)
{
  assert(parentA.team.getNumTeammates() == parentB.team.getNumTeammates());
  std::vector<const ranked*> componentTeams;
  componentTeams.push_back(&parentA);
  componentTeams.push_back(&parentB);
  std::vector<fpType> numCTeam(2, 0.0);
  ranked_team crossedTeam;
  size_t maxGeneration = std::max(parentA.generation, parentB.generation);
  size_t numTeammates = parentA.team.getNumTeammates();

  {
    team_nonvolatile& cTeam = crossedTeam.team;
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
          const pokemon_nonvolatile& candidate = parentA.team.teammate(iParentA);
          if (crossedTeam.team.isLegalAdd(candidate)) { cTeam.addPokemon(candidate); numCTeam[0]+= 1.0; break; }

          // try adding parentB's pokemon:
          const pokemon_nonvolatile& backup = parentB.team.teammate(iParentB);
          if (crossedTeam.team.isLegalAdd(backup)) { cTeam.addPokemon(backup); numCTeam[1]+= 1.0; break; }

          // try adding a random pokemon: (guaranteed to be a legal add by createRandom_single)
          pokemon_nonvolatile cTeammate = createRandom_single(crossedTeam.team);
          if (!cTeam.isLegalAdd(cTeammate)) { return createRandom(settings, numTeammates, 0); }
          cTeam.addPokemon(cTeammate);
          break;
        }
      case 1: // fully expressed parent B
        {
          // try adding parentB's pokemon first:
          const pokemon_nonvolatile& candidate = parentB.team.teammate(iParentB);
          if (crossedTeam.team.isLegalAdd(candidate)) { cTeam.addPokemon(candidate); numCTeam[TEAM_B]+= 1.0; break; }

          // try adding parentA's pokemon:
          const pokemon_nonvolatile& backup = parentA.team.teammate(iParentA);
          if (crossedTeam.team.isLegalAdd(backup)) { cTeam.addPokemon(backup); numCTeam[TEAM_A]+= 1.0; break; }

          // try adding a random pokemon: (guaranteed to be a legal add by createRandom_single)
          pokemon_nonvolatile cTeammate = createRandom_single(crossedTeam.team);
          if (!cTeam.isLegalAdd(cTeammate)) { return createRandom(settings, numTeammates, 0); }
          cTeam.addPokemon(cTeammate);
          break;
        }
        break;
      case 2: // partially expressed A or B
      case 3:
        {
          // generate a crossover of parentA and parentB's pokemon:
          pokemon_nonvolatile candidate = crossover_single(parentA.team.teammate(iParentA), parentB.team.teammate(iParentB));
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
  crossedTeam.getSkill() = trueSkill::teamRank(componentTeams.begin(), componentTeams.end(), numCTeam.begin(), 1.0, settings);
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





pokemon_nonvolatile ranked_team::createRandom_single(const team_nonvolatile& cTeam, size_t iReplace)
{
  pokemon_nonvolatile cPokemon;

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





void ranked_team::mutate_single(ranked_team& cRankteam, size_t iTeammate, size_t numMutations)
{
  pokemon_nonvolatile& cPokemon = cRankteam.team.teammate(iTeammate);

  // if it's requested that we perform more than 8 unique mutations, just create a random teammate
  if (numMutations > 8) { cPokemon = cPokemon = createRandom_single(cRankteam.team, iTeammate); return; }

  boost::array<bool, 9> isMutated;
  isMutated.assign(false);
  
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

    switch(mutationType)
    {
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
}// endOf mutate_single





pokemon_nonvolatile ranked_team::crossover_single(
  const pokemon_nonvolatile& parentA, 
  const pokemon_nonvolatile& parentB)
{
  const pokemon_nonvolatile& basePokemon = (((rand()%2)==TEAM_A)?parentA:parentB);
  const pokemon_nonvolatile& otherPokemon = (&basePokemon==&parentA)?parentB:parentA;

  // basePokemon maintains species, ability, moveset
  pokemon_nonvolatile crossedPokemon(basePokemon);

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





void ranked_team::randomSpecies(const team_nonvolatile& cTeam, pokemon_nonvolatile& cPokemon, size_t iReplace)
{
  bool revalidate = cPokemon.pokemonExists();
  bool isSuccessful = false;
  size_t iSpecies = (rand() % pkdex->getPokemon().size()) - 1;
  // find a pokemon with an existing movelist (non orphan)
  for (size_t iNSpecies = 0; iNSpecies != pkdex->getPokemon().size(); ++iNSpecies)
  {
    iSpecies = (iSpecies + 1) % pkdex->getPokemon().size();
    const pokemon_base& candidateBase = pkdex->getPokemon()[iSpecies];

    // don't include a species already on the team:
    if ((iReplace == SIZE_MAX) && !cTeam.isLegalAdd(candidateBase)) { continue; }
    else if (!cTeam.isLegalSet(iReplace, candidateBase)) { continue; }
    // don't include any pokemon species with lost children: (no moves, no ability, no types)
    if (candidateBase.lostChild == true) { continue; }
    
    isSuccessful = true;
    cPokemon.setBase(pkdex->getPokemon()[iSpecies]);
    break;
  }
  if (!isSuccessful)
  {
    // something horrible happened, do not attempt to randomize the species. No need to revalidate if this occurs
    assert(false && "Failed to generate random species!");
    return;
  }

  if (revalidate)
  {
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
      const pokemon_base& cBase = cPokemon.getBase();
      bool isMatched = std::binary_search(cBase.abilities.begin(), cBase.abilities.end(), &cPokemon.getAbility());
      if (!isMatched)
      {
        // and update with a new ability if it is
        randomAbility(cPokemon);
      }
    }

    assert(cPokemon.getNumMoves() > 0);
  } // endOf revalidation
} // endOf randomSpecies





void ranked_team::randomAbility(pokemon_nonvolatile& cPokemon)
{
  const pokemon_base& cBase = cPokemon.getBase();

  size_t iAbility = rand() % cBase.getNumAbilities();
  for (size_t iNAbility = 0; iNAbility != cBase.getNumAbilities(); ++iNAbility)
  {
    if (cBase.getAbility(iAbility).isImplemented()) { break; }

    iAbility = (iAbility + 1) % cBase.getNumAbilities();
  }
    
  // if the ability does not have a script implemented:
  if (!cBase.getAbility(iAbility).isImplemented())
  {
    cPokemon.setNoAbility();
  }
  else
  {
    cPokemon.setAbility(cBase.getAbility(iAbility));
  }
} // endOf randomAbility





void ranked_team::randomNature(pokemon_nonvolatile& cPokemon)
{
  size_t iNature = rand() % pkdex->getNatures().size();
  cPokemon.setNature(pkdex->getNatures()[iNature]);
}





void ranked_team::randomItem(pokemon_nonvolatile& cPokemon)
{
  size_t iNSize = (pkdex->getItems().size() + 1);
  size_t iItem = (rand() % iNSize) - 1;
  for (size_t iNItem = 0; iNItem != iNSize; ++iNItem)
  {
    // in this case, we choose no item:
    if (iItem == SIZE_MAX) { break; } 

    if (pkdex->getItems()[iItem].isImplemented()) { break; }

    iItem = ((iItem + 2) % iNSize) - 1;
  }

  // if the item does not have a script implemented or we explicitly chose no item:
  if ((iItem == SIZE_MAX) || (!pkdex->getItems()[iItem].isImplemented()))
  {
    cPokemon.setNoInitialItem();
  }
  else
  {
    cPokemon.setInitialItem(pkdex->getItems()[iItem]);
  }
} // endOf randomItem





void ranked_team::randomIV(pokemon_nonvolatile& cPokemon, size_t numIVs)
{
  boost::array<bool, 6> isValid;
  isValid.assign(true);
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





void ranked_team::randomEV(pokemon_nonvolatile& cPokemon)
{
  boost::array<unsigned int, 6> tempEV;
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





void ranked_team::randomMove(pokemon_nonvolatile& cPokemon, size_t numMoves)
{
  const std::vector<size_t>& cMovelist = cPokemon.getBase().movelist;
  std::vector<bool> isValid(cMovelist.size(), true);
  size_t validMoves = cMovelist.size();

  // remove duplicate moves from the valid move array:
  for (size_t iMove = 0; iMove != cPokemon.getNumMoves(); ++iMove)
  {
    size_t iBase = &cPokemon.getMove_base(iMove + AT_MOVE_0) - &pkdex->getMoves().front();

    for (size_t iValidMove = 0, iValidSize = cMovelist.size(); iValidMove != iValidSize; ++iValidMove)
    {
      if (iBase == cMovelist[iValidMove]) { isValid[iValidMove] = false; validMoves--; break; }
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

      const move& possibleMove = pkdex->getMoves().at(cMovelist.at(iMove));

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
    move_nonvolatile cMove(pkdex->getMoves().at(cMovelist.at(iMove)));

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





void ranked_team::randomGender(pokemon_nonvolatile& cPokemon)
{
  unsigned int gender = rand() % 3;
  // TODO: allowed genders based on species
  cPokemon.setSex(gender);
}
