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

#include "../inc/engine.h"
#include "../inc/game.h"
#include "../inc/genetic.h"
#include "../inc/roulette.h"
#include "../inc/init_toolbox.h"

#include "../inc/ranked_team.h"

const std::string RankedTeam::header = "PKAIR1";

#define DELETECOMPONENTTEAMS(componentTeams, createdTeams) \
for (size_t iComponentTeam = 0; iComponentTeam != componentTeams.size(); ++iComponentTeam) \
{ \
  const RankedTeam* cComponentTeam = (RankedTeam*)componentTeams[iComponentTeam]; \
  if (createdTeams[iComponentTeam] == true) { delete cComponentTeam; } \
}





RankedTeam::RankedTeam(const TeamNonVolatile& cTeam, size_t _generation, const trueSkillSettings& cSettings)
  : Ranked(_generation, cSettings),
  nv_(cTeam),
  hash(),
  rankPoints(),
  numPlies(),
  numMoves()
{
  skill() = TrueSkill(1, cSettings);
  hash.fill(Ranked::defaultHash);
  numPlies.fill(0);
  rankPoints.fill(0);
  for (size_t iTeammate = 0; iTeammate != nv_.getMaxNumTeammates(); ++iTeammate)
  {
    numMoves[iTeammate].fill(0);
  }
};





RankedTeam::RankedTeam(const RankedTeam& other)
  : Ranked(other),
  nv_(other.nv_),
  hash(other.hash),
  rankPoints(other.rankPoints),
  numPlies(other.numPlies),
  numMoves(other.numMoves)
{
};





std::ostream& operator <<(std::ostream& os, const RankedTeam& tR)
{
  os << 
    "" << std::setw(20) << tR.team_.getName().substr(0,20) << " ";

  // print a few characters from each teammate:
  size_t teammateStringSize = 0;
  size_t sizeAccumulator = 0;
  for (size_t iTeammate = 0; iTeammate != tR.team_.getNumTeammates(); ++iTeammate)
  {
    teammateStringSize = (24 - sizeAccumulator) / (tR.team_.getNumTeammates() - iTeammate);

    const std::string& baseName = tR.team_.teammate(iTeammate).getBase().getName();
    os << baseName.substr(0, teammateStringSize);

    sizeAccumulator += std::min(baseName.size(), teammateStringSize);
  }
  while (sizeAccumulator < 24) { os << " "; sizeAccumulator++; };

  size_t prevPrecision = os.precision();
  os.precision(6);
  os <<
    " g= " << std::setw(3) << std::right << tR.getGeneration() <<
    " m= " << std::setw(7) << tR.skill().getMean() <<
    " s= " << std::setw(7) << tR.skill().getStdDev() <<
    " w= " << std::setw(7) << std::left << tR.getNumWins() << 
    " / " << std::setw(7) << std::right << (tR.getNumGamesPlayed());
  os.precision(prevPrecision);
  
  return os;
}





void RankedTeam::output(std::ostream& oFile, bool printHeader) const
{
  // header:
  if (printHeader)
  {
    oFile << header << "\n";
  };

  // team hash:
  oFile << std::hex << hash() << std::dec << "\n";
  for (size_t iTeammate = 0; iTeammate != nv_.getNumTeammates(); ++iTeammate)
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
  Ranked::output(oFile);

  // the rest of this section is exactly like a normal output team method, except with a different header
  nv_.output(oFile);
} // endOf outputRankedTeam





bool RankedTeam::input(const std::vector<std::string>& lines, size_t& iLine)
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
  if (!Ranked::input(lines, iLine)) { return false; }

  // input actual team:
  if (!nv_.input(lines, iLine)) { return false; }

  return true;
} // endOf inputTeam (ranked)





void RankedTeam::generateHash(bool generateSubHashes)
{
  if (generateSubHashes)
  {
    // generate hashes for each of the pokemon:
    for (size_t iTeammate = 0; iTeammate < nv_.getNumTeammates(); ++iTeammate)
    {
      hash[iTeammate + 1] = nv_.teammate(iTeammate).hash();
    }
  }

  hash[0] = nv_.hash();
};





void RankedTeam::defineNames()
{
  size_t numPokemon = team_.getNumTeammates();
  // set name of created team based on hash:
  {
    std::ostringstream tName(std::ostringstream::out);
    tName << "_" << numPokemon << "-x" << std::setfill('0') << std::setw(16) << std::hex << hash();
    team_.setName(tName.str());
  }

  // set names of created pokemon based on hashes:
  for (size_t iTeammate = 0; iTeammate < numPokemon; ++iTeammate)
  {
    PokemonNonVolatile& cPokemon = team_.teammate(iTeammate);

    std::ostringstream tName(std::ostringstream::out);
    tName << "" << iTeammate << numPokemon << "-x" << std::setfill('0') << std::setw(16) << std::hex << getTeammateHash(iTeammate);
    cPokemon.setName(tName.str());
  }
} // endOf defineNames





size_t RankedTeam::update(const Game& cGame, const TrueSkillTeam& cTeam, size_t iTeam)
{
  size_t numUpdates = 0;
  const std::vector<GameResult>& gameResults = cGame.getGameResults();

  // update ranked vars:
  Ranked::update(cGame, cTeam, iTeam);
  BOOST_FOREACH(Ranked* componentTeam, cTeam.subTeams)
  {
    componentTeam->update(cGame, cTeam, iTeam);
  }

  // update ranked_team specific vars:
  BOOST_FOREACH(const GameResult& cGameResult, gameResults)
  {
    const std::array<std::array<uint32_t, 5>, 6>& cTeamMoves = cGameResult.moveUse[iTeam];
    const std::array<uint32_t, 6>& cTeamRanks = cGameResult.ranking[iTeam];
    const std::array<uint32_t, 6>& cTeamPlies = cGameResult.participation[iTeam];
    for (size_t iTeammate = 0; iTeammate != team_.getNumTeammates(); ++iTeammate)
    {
      const std::array<uint32_t, 5>& cTeammateMoves = cTeamMoves[iTeammate];

      // main team:
      for (size_t iMove = 0, iMoveSize = team_.teammate(iTeammate).getNumMoves(); iMove != iMoveSize; ++iMove)
      {
        // add number of a particular action this pokemon used during the match to total
        numMoves[iTeammate][iMove] += cTeammateMoves[iMove];
          
      }
      // add number of rounds teammate was in the game to total:
      numPlies[iTeammate] += cTeamPlies[iTeammate];
      // add rankPoints:
      rankPoints[iTeammate] += 
        team_.getNumTeammates() - cTeamRanks[iTeammate];
    }
    numUpdates++;

    // subteams:
    size_t iComponent = 0;
    BOOST_FOREACH(RankedTeam* componentTeam, cTeam.subTeams)
    {
      const std::array<size_t, 6>& correspondence = cTeam.correspondencies[iComponent];

      for (size_t iOTeammate = 0; iOTeammate != componentTeam->team_.getNumTeammates(); ++iOTeammate)
      {
        assert(correspondence[iOTeammate] != SIZE_MAX);

        const std::array<uint32_t, 5>& cTeammateMoves = cTeamMoves[correspondence[iOTeammate]];
          
        // increment plies:
        componentTeam->numPlies[iOTeammate] += cTeamPlies[correspondence[iOTeammate]];
        // increment rankPoints: (weighted by total team's numTeammates)
        componentTeam->rankPoints[iOTeammate] += 
          team_.getNumTeammates() - cTeamRanks[correspondence[iOTeammate]];

        // increment moves:
        for (size_t iMove = 0, iMoveSize = componentTeam->team_.teammate(iOTeammate).getNumMoves(); iMove != iMoveSize; ++iMove)
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





RankedTeam RankedTeam::selectRandom(
  const trueSkillSettings& settings,
  const std::array<std::vector<RankedTeam>, 6>& league, 
  size_t numPokemon)
{
  std::vector<const Ranked*> componentTeams;
  std::vector<bool> createdTeams;
  std::vector<fpType> numCTeam;
  RankedTeam cRankTeam;
  TeamNonVolatile& cTeam = cRankTeam.team_;
  size_t maxGeneration = 0;

  if (numPokemon > 1)
  {
    size_t numPokemonLeft = numPokemon;
    bool isSuccessful = false;
    for (size_t numTries = 0; (numTries != MAXTRIES) && (!isSuccessful); ++numTries)
    {
      // select a random team either via roulette or completely randomly:
      std::array<size_t, 2> iRandomTeam = selectRandom_single(cRankTeam, league, std::min(numPokemonLeft, numPokemon - 1));

      // if we were unable to generate a team:
      const RankedTeam* targetTeam = (iRandomTeam[0] == SIZE_MAX || iRandomTeam[1] == SIZE_MAX)
        // generate a random subsection of the team that isn't in the league:
        ? new RankedTeam(createRandom(settings, numPokemonLeft, 0))
        // or select a subsection from a smaller league
        : &league[iRandomTeam[0]][iRandomTeam[1]];

      // push back a new component team
      componentTeams.push_back(targetTeam);
      numCTeam.push_back(0);

      // add collected pokemon to cRankTeam
      for (size_t iTeammate = 0, iSize = targetTeam->team_.getNumTeammates(); iTeammate != iSize; ++iTeammate)
      {
        const PokemonNonVolatile& cTeammate = targetTeam->team_.teammate(iTeammate);

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
  cRankTeam.skill() = TrueSkill::teamRank(componentTeams.begin(), componentTeams.end(), numCTeam.begin(), 1.0, settings);
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





std::array<size_t, 2> RankedTeam::selectRandom_single(
  const RankedTeam& existing,
  const std::array<std::vector<RankedTeam>, 6>& league,
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
    const std::vector<RankedTeam>& cLeague = league[iLeague];

    // no possible matches
    if (!allowLess && cLeague.empty()) { break; }
    // are there any teams in this league at all?
    else if (cLeague.empty()) { iLeague = (iLeague + 1) % numPokemon; continue; }

    // select teammate, weighted by mean ability.
    // This method also determines if the team is a legal add
    iTeam = roulette<RankedTeam, sortByMean_noDuplicates>::select(cLeague, sortByMean_noDuplicates(existing));

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
    std::array<size_t, 2> failedResult = {{ SIZE_MAX, SIZE_MAX }};
    return failedResult;
  }

  std::array<size_t, 2> successfulResult = {{ iLeague, iTeam }};
  return successfulResult;
} // endOf selectRandom_single





RankedTeam RankedTeam::createRandom(
  const trueSkillSettings& settings,
  size_t numPokemon, 
  size_t _generation)
{
  // sets team, hash, trueskill, NOT hash or name
  RankedTeam cRankteam(createRandom(numPokemon), _generation, settings);

  // hash created pokemon:
  cRankteam.generateHash();

  cRankteam.defineNames();

  cRankteam.stateSaved = false;

  return cRankteam;
} //endOf createRandom 







RankedTeam RankedTeam::mutate(
  const trueSkillSettings& settings,
  const std::array<std::vector<RankedTeam>, 6>& league, 
  const RankedTeam& parent,
  size_t numMutations)
{
  RankedTeam mutatedTeam(parent);

  size_t numTeammates = mutatedTeam.team_.getNumTeammates();
  size_t maxGeneration = mutatedTeam.getGeneration();

  // an array of which pokemon have been seeded from which team. Used for averaging trueskill scores
  std::vector<const Ranked*> componentTeams;
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
      std::array<size_t, 2> iRandomTeam = selectRandom_single(mutatedTeam, league, (numTeammates - 1), true);

      // if we were unable to generate a team:
      const RankedTeam targetTeam = (iRandomTeam[0] == SIZE_MAX || iRandomTeam[1] == SIZE_MAX)
        // generate a random subsection of the team
        ? createRandom(settings, (rand() % (numTeammates - 1)) + 1, 0)
        // or select a subsection from a smaller league
        : league[iRandomTeam[0]][iRandomTeam[1]];

      // push back a new component team
      componentTeams.push_back(&targetTeam);
      numCTeam.push_back(0.0);

      // add elements from targetTeam to mutatedTeam, but don't add an element over an already swapped teammate:
      std::vector<bool> isSwapped(numTeammates, false);
      for (size_t iOTeammate = 0; iOTeammate != targetTeam.team_.getNumTeammates(); ++iOTeammate)
      {
        // find an acceptable target swap index:
        size_t iTarget = rand() % numTeammates;
        while (isSwapped[iTarget])
        {
          iTarget = (iTarget + 1) % numTeammates;
        };

        const PokemonNonVolatile& swappedTeammate = targetTeam.team_.teammate(iOTeammate);

        // determine if pokemon is legal add: (selectRandom_single should guarantee this)
        if (!mutatedTeam.team_.isLegalAdd(swappedTeammate)) { continue; }
        
        // add pokemon if legal:
        mutatedTeam.team_.setPokemon(iTarget, swappedTeammate);

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
      
      mutatedTeam.team_.setLeadPokemon(iSwap);
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
  mutatedTeam.skill() = TrueSkill::teamRank(componentTeams.begin(), componentTeams.end(), numCTeam.begin(), 1.0, settings);
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