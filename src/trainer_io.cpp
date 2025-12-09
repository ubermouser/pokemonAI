#include "../inc/old_trainer.h"

#include <iomanip>
#include <iostream>
#include <assert.h>
#include <unordered_map>

#include <boost/foreach.hpp>
#include <boost/math/special_functions/fpclassify.hpp>

#include <boost/filesystem/path.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/fstream.hpp>

#include "../inc/engine.h"
#include "../inc/pkIO.h"

template <class data_t>
void saveElements(
  const boost::filesystem::path& pPath, 
  std::vector<data_t> data, 
  size_t& numSaved, 
  size_t& numUpdated,
  size_t& numFailed);

template<class result_t>
void printRankedStatistics(const result_t& cR);

template<class dataType>
void printLeaderboard(const std::vector<dataType>& data, size_t numToPrint);

template<class dataType, class result_t>
void calculateRankedDescriptiveStatistics(const std::vector<dataType>& data, result_t& cR);





template<class result_t>
void printRankedStatistics(const result_t& cResult)
{
  std::clog 
    <<   " rank:   " << std::setw(10) << std::right <<  cResult.mins[0] 
    << " |-- " << std::setw(10) << std::left << cResult.averages[0]
    << " --| " << std::setw(10) << cResult.maxes[0]
    << " , stdDev= " << std::setw(10) << std::right << cResult.stdDevs[0]
    << "\n mean:   " << std::setw(10) << std::right << cResult.mins[1]
    << " |-- " << std::setw(10) << std::left << cResult.averages[1]
    << " --| " << std::setw(10) << cResult.maxes[1]
    << " , stdDev= " << std::setw(10) << std::right << cResult.stdDevs[1]
    << "\n stdDev: " << std::setw(10) << std::right << cResult.mins[2]
    << " |-- " << std::setw(10) << std::left << cResult.averages[2]
    << " --| " << std::setw(10) << cResult.maxes[2]
    << " , stdDev= " << std::setw(10) << std::right << cResult.stdDevs[2]
    << "\n plies:  " << std::setw(10) << std::right << cResult.mins[3]
    << " |-- " << std::setw(10) << std::left << cResult.averages[3]
    << " --| " << std::setw(10) << cResult.maxes[3]
    << " , stdDev= " << std::setw(10) << std::right << cResult.stdDevs[3]
    << "\n games:  " << std::setw(10) << std::right << cResult.mins[4]
    << " |-- " << std::setw(10) << std::left << cResult.averages[4]
    << " --| " << std::setw(10) << cResult.maxes[4]
    << " , stdDev= " << std::setw(10) << std::right << cResult.stdDevs[4]
    << "\n wins:   " << std::setw(10) << std::right << cResult.mins[5]
    << " |-- " << std::setw(10) << std::left << cResult.averages[5]
    << " --| " << std::setw(10) << cResult.maxes[5]
    << " , stdDev= " << std::setw(10) << std::right << cResult.stdDevs[5]
    << "\n draws:  " << std::setw(10) << std::right << cResult.mins[6]
    << " |-- " << std::setw(10) << std::left << cResult.averages[6]
    << " --| " << std::setw(10) << cResult.maxes[6]
    << " , stdDev= " << std::setw(10) << std::right << cResult.stdDevs[6]
    << "\n";
};

template<class dataType>
void printLeaderboard(const std::vector<dataType>& data, size_t numToPrint)
{
  numToPrint = std::min(numToPrint, data.size());
  for (size_t iData = 0; iData != std::min(numToPrint, data.size()); ++iData)
  {
    const dataType& cData = data[iData];
    if (cData.getNumGamesPlayed() == 0) { numToPrint++; continue; }

    std::clog 
      << std::setw(2) << iData 
      << ": " << cData
      << "\n";
  }
};

void Trainer::printNetworkStatistics(size_t numMembers, const networkTrainerResult& cResult) const
{
  size_t numToPrint = std::min(numMembers, networks.size());
  std::clog << "---- DESCRIPTIVE STATISTICS OF NETWORKS ----\n";
  if (numToPrint > 0) 
  {
    printRankedStatistics(cResult);
    std::clog 
      <<   " MSE:    " << std::setw(10) << std::right << cResult.meanSquaredError[2] 
      << " |-- " << std::setw(10) << std::left << cResult.meanSquaredError[0]
      << " --| " << std::setw(10) << cResult.meanSquaredError[3]
      << " , stdDev= " << std::setw(10) << std::right << cResult.meanSquaredError[1]
      << "\n";
  }
  // print the top numMembers elements of current league:
  std::clog 
    << "-------- TOP " << std::setw(2) << numToPrint
    << " MEMBERS OF NETWORKS --------\n";
  printLeaderboard(networks, numToPrint);
  // print gauntlet
  if (trialNet != NULL)
  {
    std::clog
      << "--: " << *trialNet
      << "\n";
  }
  // print bias evaluators, if we're doing that:
  for (size_t iEval = 0; iEval != evaluators.size(); ++iEval)
  {
    const RankedEvaluator& cEval = evaluators[iEval];
    std::clog
      << "--: " << cEval
      << "\n";
  }
  std::clog << "--------------------------------------------\n";

  std::clog.flush();
};

void Trainer::printLeagueStatistics(size_t iLeague, size_t numMembers, const TrainerResult& cResult) const
{
  const std::vector<RankedTeam>& cLeague = leagues[iLeague];

  size_t numToPrint = std::min(numMembers, cLeague.size());
  if (numToPrint == 0) { return; }

  // descriptive stats:
  std::clog << "---- DESCRIPTIVE STATISTICS OF LEAGUE " << iLeague + 1 << " ----\n";
  printRankedStatistics(cResult);
  std::clog 
    << " most popular pokemon is \"" << cResult.highestPokemon->getName()
    << "\" with " << cResult.highestPokemonCount
    << " showings\n most popular move is \"" << cResult.highestMove->getName()
    << "\" with " << cResult.highestMoveCount 
    << " showings\n most popular ability is \"" << cResult.highestAbility->getName()
    << "\" with " << cResult.highestAbilityCount
    << " showings\n most popular item is \"" << cResult.highestItem->getName()
    << "\" with " << cResult.highestItemCount 
    << " showings\n most popular type is \"" << cResult.highestType->getName()
    << "\" with " << cResult.highestTypeCount
    << " showings\n most popular nature is \"" << cResult.highestNature->getName()
    << "\" with " << cResult.highestNatureCount
    << " showings\n";

  // print the top numMembers elements of the league:
  std::clog 
    <<  "-------- TOP " <<  std::setw(2) << numToPrint
    << " MEMBERS OF LEAGUE " << iLeague + 1
    << " --------\n";
  printLeaderboard(cLeague, numMembers);
  // print gauntlet
  if (trialTeam != NULL)
  {
    std::clog
      << "--: " << *trialTeam
      << "\n";
  }
  std::clog << "--------------------------------------------\n";

  std::clog.flush();
} // printLeagueStatistics





template<class dataType, class result_t>
void calculateRankedDescriptiveStatistics(const std::vector<dataType>& data, result_t& cR)
{
  std::array< std::vector<fpType> , 7> aggData;
  // averages:
  cR.averages.fill(0.0);
  // stdDevs:
  cR.stdDevs.fill(0.0);
  // minimums:
  cR.mins.fill(std::numeric_limits<fpType>::infinity());
  // maximums:
  cR.maxes.fill(-std::numeric_limits<fpType>::infinity());

  // reserve space for descriptives:
  BOOST_FOREACH(std::vector<fpType>& cAggData, aggData)
  {
    cAggData.reserve(data.size());
  }

  // collect vars:
  BOOST_FOREACH(const dataType& cData, data)
  {
    // games:
    aggData[4].push_back(cData.getNumGamesPlayed());
    if (mostlyEQ(aggData[4].back(), 0.0)) { continue; }

    const TrueSkill& cSkill = cData.getSkill();
    // rank:
    aggData[0].push_back(cSkill.getRank());
    // mean:
    aggData[1].push_back(cSkill.getMean());
    // stdDev:
    aggData[2].push_back(cSkill.getStdDev());
    // plies:
    aggData[3].push_back(cData.getAveragePliesPerGame());
    // wins:
    aggData[5].push_back(cData.getNumWins() / aggData[4].back());
    // draws:
    aggData[6].push_back((cData.getNumDraws() + cData.getNumTies()) / aggData[4].back());
  }

  // calculate descriptives:
  size_t iData = 0;
  BOOST_FOREACH(std::vector<fpType>& cAggData, aggData)
  {
    // finalize mean:
    if (cAggData.size() > 0)
    {
      BOOST_FOREACH(const fpType& cValue, cAggData)
      {
        // add to average accumulator:
        cR.averages[iData] += cValue;

        // min/max rank:
        if (cValue < cR.mins[iData]) { cR.mins[iData] = cValue; }
        if (cValue > cR.maxes[iData]) { cR.maxes[iData] = cValue; }

      }

      cR.averages[iData] /= (fpType)cAggData.size();

      // standard deviations:
      BOOST_FOREACH(const fpType& cValue, cAggData)
      {
        cR.stdDevs[iData] += pow(cValue - cR.averages[iData], 2);
      }

      // finalize stdDev:
      cR.stdDevs[iData] /= (fpType)cAggData.size();
      cR.stdDevs[iData] = sqrt(cR.stdDevs[iData]);

      assert(!boost::math::isnan(cR.stdDevs[iData]) || !boost::math::isnan(cR.averages[iData]));
    }
    else
    {
      cR.averages[iData] = 0;
      cR.stdDevs[iData] = 0;
      cR.mins[iData] = 0;
      cR.maxes[iData] = 0;
    }

    ++iData;
  }
}

void Trainer::calculateDescriptiveStatistics(networkTrainerResult& cR) const
{
  // calculate ranked vars:
  calculateRankedDescriptiveStatistics(networks, cR);
  // initialize:
  cR.meanSquaredError.fill(0.0);
  cR.meanSquaredError[2] = std::numeric_limits<fpType>::infinity();
  cR.meanSquaredError[3] = -std::numeric_limits<fpType>::infinity();
  // calculate mean squared errors:
  std::vector<fpType> aggData;
  aggData.reserve(networks.size());

  // collect vars:
  BOOST_FOREACH(const ranked_neuralNet& cNetwork, networks)
  {
    if (mostlyEQ(cNetwork.getNumUpdates(), 0.0)) { continue; }
    aggData.push_back(cNetwork.getMeanSquaredError());
  }

  if (aggData.empty())
  {
    cR.meanSquaredError.fill(0.0);
    return;
  }

  // calculate descriptives:
  BOOST_FOREACH(const fpType& cValue, aggData)
  {
    // add to average accumulator:
    cR.meanSquaredError[0] += cValue;

    // min/max rank:
    if (cValue < cR.meanSquaredError[2]) { cR.meanSquaredError[2] = cValue; }
    if (cValue > cR.meanSquaredError[3]) { cR.meanSquaredError[3] = cValue; }
  }

  cR.meanSquaredError[0] /= (fpType)aggData.size();

  // standard deviations:
  BOOST_FOREACH(const fpType& cValue, aggData)
  {
    cR.meanSquaredError[1] += pow(cValue - cR.meanSquaredError[0], 2);
  }

  // finalize stdDev:
  cR.meanSquaredError[1] /= (fpType)aggData.size();
  cR.meanSquaredError[1] = sqrt(cR.meanSquaredError[1]);

  assert(!boost::math::isnan(cR.meanSquaredError[0]) || !boost::math::isnan(cR.meanSquaredError[1]));
}

void Trainer::calculateDescriptiveStatistics(size_t iLeague, TrainerResult& cR) const
{
  // initialize:
  const std::vector<RankedTeam>& cLeague = leagues[iLeague];
  // calculate ranked vars:
  calculateRankedDescriptiveStatistics(cLeague, cR);

  std::unordered_map<const PokemonBase*, size_t> pokemonCounts;
  std::unordered_map<const Ability*, size_t> abilityCounts;
  std::unordered_map<const Item*, size_t> itemCounts;
  std::unordered_map<const Move*, size_t> moveCounts;
  std::unordered_map<const Type*, size_t> typeCounts;
  std::unordered_map<const Nature*, size_t> natureCounts;
  // highest counts:
  cR.highestPokemon = PokemonBase::no_base; cR.highestPokemonCount = 0;
  cR.highestMove = Move::move_none; cR.highestMoveCount = 0;
  cR.highestItem = Item::no_item; cR.highestItemCount = 0;
  cR.highestAbility = Ability::no_ability; cR.highestAbilityCount = 0;
  cR.highestNature = Nature::no_nature; cR.highestNatureCount = 0;
  cR.highestType = Type::no_type; cR.highestTypeCount = 0;

  // foreach team:
  size_t iTeam = 0;
  BOOST_FOREACH(const RankedTeam& cRTeam, cLeague)
  {
    const TeamNonVolatile& cTeam = cRTeam.team;

    // foreach pokemon:
    for (size_t iTeammate = 0; iTeammate != cTeam.getNumTeammates(); ++iTeammate)
    {
      const PokemonNonVolatile& cTeammate = cTeam.teammate(iTeammate);

      // pokemon counts:
      if (cTeammate.pokemonExists())
      {
        const PokemonBase* cPokemon = &cTeammate.getBase();
        pokemonCounts[cPokemon]++;
        if (pokemonCounts[cPokemon] > cR.highestPokemonCount)
        { 
          cR.highestPokemon = cPokemon;
          cR.highestPokemonCount = pokemonCounts[cPokemon];
        }
      }

      // item counts:
      if (cTeammate.hasInitialItem())
      {
        const Item* cItem = &cTeammate.getInitialItem();
        itemCounts[cItem]++;
        if (itemCounts[cItem] > cR.highestItemCount)
        { 
          cR.highestItem = cItem;
          cR.highestItemCount = itemCounts[cItem];
        }
      }

      // ability counts:
      if (cTeammate.abilityExists())
      {
        const Ability* cAbility = &cTeammate.getAbility();
        abilityCounts[cAbility]++;
        if (abilityCounts[cAbility] > cR.highestAbilityCount)
        { 
          cR.highestAbility = cAbility;
          cR.highestAbilityCount = abilityCounts[cAbility];
        }
      }

      // nature counts:
      if (cTeammate.natureExists())
      {
        const Nature* cNature = &cTeammate.getNature();
        natureCounts[cNature]++;
        if (natureCounts[cNature] > cR.highestNatureCount)
        { 
          cR.highestNature = cNature;
          cR.highestNatureCount = natureCounts[cNature];
        }
      }

      // type counts:
      {
        if (&cTeammate.getBase().getType(0) != Type::no_type)
        {
          // type 1:
          const Type* cType = &cTeammate.getBase().getType(0);
          typeCounts[cType]++;
          if (typeCounts[cType] > cR.highestTypeCount)
          { 
            cR.highestType = cType;
            cR.highestTypeCount = typeCounts[cType];
          }
        }
        if (&cTeammate.getBase().getType(1) != Type::no_type)
        {
          // type 2:
          const Type* cType = &cTeammate.getBase().getType(1);
          typeCounts[cType]++;
          if (typeCounts[cType] > cR.highestTypeCount)
          { 
            cR.highestType = cType;
            cR.highestTypeCount = typeCounts[cType];
          }
        }
      }

      // foreach move:
      for (size_t iMove = 0; iMove != cTeammate.getNumMoves(); ++iMove)
      {
        const Move* cMove = &cTeammate.getMove_base(Action::move(iMove));

        // move counts:
        moveCounts[cMove]++;

        if (moveCounts[cMove] > cR.highestMoveCount)
        { 
          cR.highestMove = cMove;
          cR.highestMoveCount = moveCounts[cMove];
        }

      } // endOf foreach move
    } // endOf foreach pokemon
    iTeam++;
  } // endOf foreach team
};





template <class data_t>
void saveElements(
  const boost::filesystem::path& pPath, 
  std::vector<data_t> data, 
  size_t& numSaved, 
  size_t& numUpdated,
  size_t& numFailed)
{
  BOOST_FOREACH(data_t& cData, data)
  {
    bool isSuccessful = false;
    bool exists = false;

    if (cData.isStateSaved()) { continue; }

    // generate full path and name of file we intend to write to:
    std::string cFileName(cData.getName()); 
    cFileName.append(".txt");
    boost::filesystem::path cDataPath(pPath / cFileName);

    // determine if file exists. It will be truncated in a second
    exists = boost::filesystem::exists(cDataPath);

    boost::filesystem::ofstream cDataFile(cDataPath, std::ios::out | std::ios::binary | std::ios::trunc); // read in, binary
    if (!cDataFile.is_open() || !cDataFile.good())
    {
      if (verbose >= 5)
      {
        std::cerr << "WAR " << __FILE__ << "." << __LINE__ << 
          ": Unable to open file for binary writing at \"" << cDataPath << "\"!\n";
      }
      if (cDataFile.is_open()) { cDataFile.close(); }
      numFailed++;
      continue;
    }

    if (verbose >= 6)
    {
      std::cerr << "INF " << __FILE__ << "." << __LINE__ << 
        ": Writing out ranked object at \"" << cDataPath << "\"...\n";
    }

    // writeOut the team:
    cData.output(cDataFile);

    if (cDataFile.good())
    {
      isSuccessful = true;
    }

    // close file handle:
    cDataFile.close();

    if (!isSuccessful)
    {
      if (verbose >= 5)
      {
        std::cerr << "WAR " << __FILE__ << "." << __LINE__ << 
          ": Unable to write ranked object to disk at \"" << cDataPath << "\"!\n";
      }
      numFailed++;
      continue;
    }
    else
    {
      // assert that we were able to complete writing the team to disk:
      if (exists)
      {
        numUpdated++;
      }
      else
      {
        numSaved++;
      }
      cData.setStateSaved();
    }
  } // endOf foreach team
}

bool Trainer::saveTeamPopulation()
{
  // assure path exists:
  boost::filesystem::path pPath(teamPath);

  // determine if folder exists:
  if (!boost::filesystem::exists(pPath))
  {
    if (verbose >= 6)
    {
      std::cerr << "INF " << __FILE__ << "." << __LINE__ << 
        ": A population folder was not found at location \"" << pPath << "\", creating one...\n";
    }
    
    if (!boost::filesystem::create_directory(pPath))
    {
      std::cerr << "ERR " << __FILE__ << "." << __LINE__ << 
        ": A population folder could not be created at \"" << pPath << "\"!\n";
      return false;
    }
  }

  // determine if folder is a directory:
  if (!boost::filesystem::is_directory(pPath))
  {
    std::cerr << "ERR " << __FILE__ << "." << __LINE__ << 
      ": \"" << pPath << "\" is not a directory!\n";
    return false;
  }

  if (verbose >= 1)
  {
    std::cout << "Backing up population to disk at location \"" << teamPath << "\"...\n";
  }

  size_t numDestroyedTeams = 0;
  size_t numFailed = 0;
  size_t numTotalTeams = 0;
  size_t numTotalSavedTeams = 0;
  size_t numSavedTeams = 0;
  size_t numUpdatedTeams = 0;

  for (size_t iLeague = 0; iLeague != leagues.size(); ++iLeague)
  {
    numTotalTeams += leagues[iLeague].size();
  }

  // destroy outdated elements in directory:
  for ( boost::filesystem::directory_iterator cTeamFile(pPath), endTeamFile; cTeamFile != endTeamFile; ++cTeamFile)
  {
    // ignore directories
    if (boost::filesystem::is_directory(*cTeamFile)) { continue; }

    // make sure extension is txt, ignore all others:
    if ((cTeamFile->path().extension().compare(".txt") != 0)) { continue; }

    const std::string cTeamFileStem = cTeamFile->path().stem().string();

    // make sure extension is of the form _N-x<HASH>.txt
    if (cTeamFileStem.size() != 20) { continue; }

    // find league and hash from name of team:
    size_t iLeague;
    uint64_t hash;
    {
      std::stringstream hexNameBuf(cTeamFileStem);
      // determine league:
      hexNameBuf.seekg(1);
      iLeague = hexNameBuf.get();
      iLeague = iLeague - '0' - 1;
      // determine hash:
      hexNameBuf.seekg(4);
      if ( (iLeague > 6) || !(hexNameBuf >> std::hex >> hash )) 
      {
        std::cerr << "ERR " << __FILE__ << "." << __LINE__ << 
        ": Unable to determine league and hash from filename of \"" << cTeamFile->path() << "\"!\n";
        continue; 
      }
    }

    numTotalSavedTeams++;

    // if currently in the league array or part of a league we haven't modified yet:
    size_t iTeam = findInPopulation(iLeague, hash);
    if ((iTeam != SIZE_MAX) || (heatsCompleted[iLeague] == 0))
    {
      // DO NOT delete team:
      continue;
    }

    if (verbose >= 6)
    {
      std::cerr << "INF " << __FILE__ << "." << __LINE__ << 
        "Deleting team at \"" << cTeamFile->path() << "\"...\n";
    }

    if (!boost::filesystem::remove(cTeamFile->path()))
    {
      std::cerr << "ERR " << __FILE__ << "." << __LINE__ << 
        ": Could not delete file at \"" << cTeamFile->path() << "\"!\n";
      return false;
    }

    numDestroyedTeams++;
  } // endOf destroy outdated elements

  // add elements to directory:
  for (size_t iLeague = 0; iLeague != leagues.size(); ++iLeague)
  {
    std::vector<RankedTeam>& cLeague = leagues[iLeague];
    saveElements(pPath, cLeague, numSavedTeams, numUpdatedTeams, numFailed);
  } // endOf foreach league

  if (numFailed > 0)
  {
    std::cerr << "ERR " << __FILE__ << "." << __LINE__ << 
      "Backing up " << numFailed << 
      " of " << numTotalTeams <<
      " FAILED!\n";
    // todo: stop backup on failure mode:
  }

  if (verbose >= 0)
  {
    std::clog 
      << "Saved " << numSavedTeams 
      << " ( updated " << numUpdatedTeams
      << " ) of " << numTotalTeams
      << " teams, deleted " << numDestroyedTeams 
      << " of " << numTotalSavedTeams
      << " from \"" << teamPath << "\" !\n";
    std::clog.flush();
  }

  return true;
} // endOf savePopulation

bool Trainer::loadTeamPopulation()
{
  // assure path exists:
  boost::filesystem::path pPath(teamPath);

  // determine if folder exists:
  if (!boost::filesystem::exists(pPath))
  {
    if (verbose >= 6)
    {
      std::cerr << "INF " << __FILE__ << "." << __LINE__ << 
        ": A population folder was not found at location \"" << pPath << 
        "\", it will be created upon next backup...\n";
    }
    return true;
  }

  // determine if folder is a directory:
  if (!boost::filesystem::is_directory(pPath))
  {
    std::cerr << "ERR " << __FILE__ << "." << __LINE__ << 
      ": \"" << pPath << "\" is not a directory!\n";
    return false;
  }

  if (verbose >= 1)
  {
    std::cout << "Loading backup population from disk at location \"" << teamPath << "\"...\n";
  }

  // load all elements in directory:
  size_t numLoadedTeams = 0;
  size_t numTotalTeams = 0;
  size_t numIncorrectHashes = 0;
  for ( boost::filesystem::directory_iterator cTeamFile(pPath), endTeamFile; cTeamFile != endTeamFile; ++cTeamFile)
  {
    bool isSuccessful = false;
    // ignore directories
    if (boost::filesystem::is_directory(*cTeamFile)) { continue; }

    // make sure extension is txt, ignore all others:
    if ((cTeamFile->path().extension().compare(".txt") != 0)) { continue; }

    // TODO: determine if the file is currently in the league array:

    if (verbose >= 6)
    {
      std::cerr << "INF " << __FILE__ << "." << __LINE__ << 
        "Loading team at \"" << cTeamFile->path() << "\"...\n";
    }

    numTotalTeams++;

    // read file from disk
    RankedTeam cTeam(TeamNonVolatile(), 0, tSettings);
    isSuccessful = PkIO::inputRankedTeam(cTeamFile->path(), cTeam);

    if (!isSuccessful)
    {
      std::cerr << "ERR " << __FILE__ << "." << __LINE__ << 
        ": Unable to read team from disk at \"" << cTeamFile->path() << "\"!\n";
      continue;
    }

    cTeam.setStateSaved();

    // make sure the hashes are correct, and if they're not redo them:
    {
      uint64_t oldTeamHash = cTeam.hash();
      cTeam.generateHash();
      if (cTeam.hash() != oldTeamHash)
      {
        numIncorrectHashes++;
        // recreate the team's naming if the hash was incorrect
        cTeam.defineNames();

        if (verbose >= 5)
        {
          std::cerr << "WAR " << __FILE__ << "." << __LINE__ << 
            " Team at \"" << cTeamFile->path() << "\" hash does not match its specified team!\n";
        }
      } // endOf hash incorrect
    }

    // don't add a duplicate team:
    if (isInPopulation(cTeam)){ isSuccessful = false; }


    if (isSuccessful)
    {
      numLoadedTeams++;
      leagues[cTeam.team.getNumTeammates() - 1].push_back(cTeam);
    }
  } // endOf foreach file

  if (verbose >= 0)
  {
    std::clog << "loaded " << numLoadedTeams << 
      " of " << numTotalTeams <<
      " teams ( " << numIncorrectHashes <<
      " hashes rebuilt ) from \"" << teamPath << "\"!\n";
    std::clog.flush();
  }
  
  return true;
} // endOf loadElements
