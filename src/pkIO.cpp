#include "../inc/pkIO.h"

#include <ostream>
#include <algorithm>
#include <stdexcept>

#include <boost/function.hpp>
#include <boost/dll/shared_library.hpp>

#include "../inc/init_toolbox.h"

#define PKAI_STATIC
#include "../inc/plugin.h"
#include "../inc/pluggable.h"
#include "../inc/pokemonAI.h"
#include "../inc/orphan.h"
#include "../inc/pokemon_base.h"
#include "../inc/nature.h"
#include "../inc/move.h"
#include "../inc/item.h"
#include "../inc/ability.h"
#include "../inc/type.h"
#include "../inc/pokemon_nonvolatile.h"
#include "../inc/team_nonvolatile.h"
#undef PKAI_STATIC
#include "../inc/ranked_team.h"
#include "../inc/ranked_neuralNet.h"

//using namespace boost::extensions;

using namespace INI;
using namespace orphan;

PkIO::PkIO(class PokemonAI* _parent)
  : parent(_parent)
{
}





bool PkIO::parseArg(const std::string& input, PkIO*& target)
{
#define genericArg( commandName, argLocation) \
else if (expression.compare(#commandName) == 0)\
{\
if (tokens.size() != 2) { incorrectNumArgs( #commandName, tokens.size(), 2); return false; }\
if (!setArg(tokens.at(1), argLocation)) { incorrectArgs(#commandName, 0, 0); return false; }\
printSetArg(#commandName, argLocation);\
return true;\
}

  std::vector<std::string> tokens = tokenize(input, " ");
  if (tokens.size() == 0)
  {
    // ignore this line, contains no useful data
    return true;
  }
  std::string& expression = tokens.at(0);
  if (expression.compare("verbose") == 0)
  {
    if (tokens.size() == 1)
    {
      verbose = 6;
    }
    else if (tokens.size() == 2)
    {
      if (!setArg(tokens.at(1), verbose))
      {
        incorrectArgs("verbose", 0, 0);
        return false;
      }
    }
    else
    {
      incorrectNumArgs("verbose", tokens.size(), 2);
      return false;
    }

    printSetArg("verbosity", verbose);
    return true;
  }
  else if (expression.compare("srand") == 0)
  {
    unsigned int seed;
    if (tokens.size() != 2)
    {
      incorrectNumArgs("srand", tokens.size(), 2);
      return false;
    }
    if (tokens.at(1).compare("time") == 0)
    {
      // seed srand with time
      seed = time(NULL);
    }
    else
    {
      if (!setArg(tokens.at(1), seed))
      {
        incorrectArgs("srand", 0, 0);
        return false;
      }
    }
    printSetArg("RNG-Seed", seed);
    srand(seed);

    return true;
  }
  // GENERIC ARGS HERE:
  genericArg(gameType, target->parent->gameType) // executive inputs:
  genericArg(numThreads, target->parent->numThreads) // search inputs:
  genericArg(secondsPerMove, target->parent->secondsPerMove)
  genericArg(maxSearchDepth, target->parent->maxSearchDepth)
  genericArg(tableBinSize, target->parent->transpositionTableBinSize)
  genericArg(tableBins, target->parent->transpositionTableBins)
  genericArg(engineAccuracy, target->parent->engineAccuracy) // engine inputs:
  genericArg(gameAccuracy, target->parent->gameAccuracy)
  genericArg(maxMatches, target->parent->maxMatches) // game inputs:
  genericArg(maxPlies, target->parent->maxPlies)
  genericArg(crossoverProbability, target->parent->crossoverProbability) // genetic algorithm inputs:
  genericArg(mutationProbability, target->parent->mutationProbability)
  genericArg(seedProbability, target->parent->seedProbability)
  genericArg(minimumWorkTime, target->parent->minimumWorkTime)
  genericArg(writeOutInterval, target->parent->writeOutInterval)
  genericArg(maxGenerations, target->parent->maxGenerations)
  genericArg(teamDirectory, target->parent->teamDirectory) // team builder inputs:
  genericArg(seedTeams, target->parent->seedTeams)
  genericArg(enforceSameLeague, target->parent->enforceSameLeague)
  genericArg(networkDirectory, target->parent->networkDirectory) // network builder inputs:
  genericArg(networkPopulation, target->parent->networkPopulation)
  genericArg(seedNetworks, target->parent->seedNetworks)
  genericArg(jitterEpoch, target->parent->jitterEpoch)
  genericArg(seedNetworkProbability, target->parent->seedNetworkProbability)
  genericArg(seedDumbEvaluator, target->parent->seedDumbEvaluator)
  genericArg(seedRandomEvaluator, target->parent->seedRandomEvaluator)
  genericArg(plannerTemperature, target->parent->plannerTemperature) // stochastic planner inputs:
  genericArg(plannerExploration, target->parent->plannerExploration)
  genericArg(plannerExperienceTaps, target->parent->expSettings.numTaps) // directed planner inputs:
  genericArg(plannerExperienceDecay, target->parent->expSettings.decay)
  genericArg(plannerExperienceExtrapolation, target->parent->expSettings.extrapolation)
  genericArg(learningLambda, target->parent->netSettings.lambda) // learning inputs:
  genericArg(learningGamma, target->parent->netSettings.gamma)
  genericArg(learningRate, target->parent->netSettings.learningRate)
  genericArg(learningMomentum, target->parent->netSettings.momentum)
  genericArg(jitterSize, target->parent->netSettings.jitterMax)
  genericArg(numRollouts, target->parent->numRollouts)
  genericArg(trueskill_initialMean, target->parent->tSettings.initialMean) // trueSkill inputs:
  genericArg(trueskill_initialStdDev, target->parent->tSettings.initialStdDev)
  genericArg(trueskill_dPerformance, target->parent->tSettings.performanceStdDev)
  genericArg(trueskill_drawProbability, target->parent->tSettings.drawProbability)
  genericArg(trueskill_dynamicsFactor, target->parent->tSettings.dynamicsFactor)
  genericArg(movesInput, target->parent->pokedex_config.movesPath_) // library inputs:
  genericArg(typesInput, target->parent->pokedex_config.typesPath_)
  genericArg(naturesInput, target->parent->pokedex_config.naturesPath_)
  genericArg(itemsInput, target->parent->pokedex_config.itemsPath_)
  genericArg(abilitiesInput, target->parent->pokedex_config.abilitiesPath_)
  genericArg(pokemonInput, target->parent->pokedex_config.pokemonPath_)
  genericArg(movelistInput, target->parent->pokedex_config.movelistsPath_)
  genericArg(pluginInput, target->parent->pokedex_config.pluginsPath_)
  // NON-GENERIC ARGS HERE:
  else if (expression.compare("teamPopulations") == 0)
  {
    if (tokens.size() != (1 + target->parent->teamPopulations.size()))
    {
      incorrectNumArgs("teamPopulations", tokens.size(), 1 + target->parent->teamPopulations.size());
      return false;
    }

    for (size_t iLeague = 0; iLeague < target->parent->teamPopulations.size(); ++iLeague)
    {
      size_t cLeagueSize = SIZE_MAX;
      if (!setArg(tokens.at(iLeague + 1), cLeagueSize)) 
      { 
        incorrectArgs("teamPopulations", 0, iLeague + 1); 
        return false;
      }
      target->parent->teamPopulations[iLeague] = cLeagueSize;
    }

    printSetArg("teamPopulations", input.substr(sizeof("teamPopulations"), input.size()));
    return true;
  }
  else if (expression.compare("networkLayers") == 0)
  {
    if (tokens.size() < 2)
    {
      incorrectNumArgs("networkLayers", tokens.size(), 2);
      return false;
    }

    target->parent->networkLayers.clear();
    target->parent->networkLayers.reserve(tokens.size() - 1);
    for (size_t iLayer = 0; iLayer < tokens.size() - 1; ++iLayer)
    {
      size_t numNeurons = SIZE_MAX;
      if (!setArg(tokens.at(iLayer + 1), numNeurons)) 
      { 
        incorrectArgs("networkLayers", 0, iLayer + 1); 
        return false;
      }
      target->parent->networkLayers.push_back(numNeurons);
    }

    printSetArg("networkLayers", input.substr(sizeof("networkLayers"), input.size()));
    return true;
  }
  else if (expression.compare("networkInput") == 0)
  {
    if (tokens.size() == 2)
    {
      std::string networkLocation;
      if (!setArg(tokens.at(1), networkLocation))
      {
        incorrectArgs("networkInput", 0, 0);
        return false;
      }
      printSetArg("networkInput", networkLocation);
      target->input_networks.push_back(networkLocation);
      return true;
    }
    else
    {
      incorrectNumArgs("teamInput", tokens.size(), 2);
      return false;
    }
  }
  else if (expression.compare("teamInput") == 0)
  {
    if (tokens.size() == 2)
    {
      std::string teamLocation;
      if (!setArg(tokens.at(1), teamLocation))
      {
        incorrectArgs("teamInput", 0, 0);
        return false;
      }
      printSetArg("teamInput", teamLocation);
      target->input_teams.push_back(teamLocation);
      return true;
    }
    else
    {
      incorrectNumArgs("teamInput", tokens.size(), 2);
      return false;
    }
  }
  // error case:
  else
  {
    std::cerr << "ERR " << __FILE__ << "." << __LINE__ << 
      ": Unrecognized argument \"" << tokens.at(0) << "\".\n Run with argument \"help\" for information about operation of this program.\n";
    return false;
  }

#undef genericArg
  return true;
} // end of parseArg





bool PkIO::inputRankedNetwork(const boost::filesystem::path& path, ranked_neuralNet& cNet)
{
  std::vector<std::string> lines;
  {
    std::string inputBuffer;

    if (loadFileToString(path, "PKNTR", inputBuffer) != true)
    {
      return false;
    }

    //tokenize by line endings
    lines = tokenize(inputBuffer, "\n\r");
  }

  size_t iLine = 0;
  return cNet.input(lines, iLine);
} // endOf inputRankedTeam

bool PkIO::inputPlayerNetwork(const boost::filesystem::path& path, neuralNet& cResult)
{
  /*
   * Header data:
   * PKAIE
   * <team name>\n
   * <nickname> <species> <level> <item> <gender> <ability> <nature> <hp.type> <hp.dmg> <move 1> <move 2> <move 3> <move 4> <atk.iv> <spatck.iv> <def.iv> <spdef.iv> <spd.iv> <hp.iv> <atk.ev> <spatck.ev> <def.ev> <spdef.ev> <spd.ev> <hp.ev>
   */

  std::vector<std::string> lines;
  size_t iLine = 0;
  {
    std::string inputBuffer;

    if (loadFileToString(path, "PKN", inputBuffer) != true)
    {
      return false;
    }

    //tokenize by line endings
    lines = tokenize(inputBuffer, "\n\r");
  }

  // this is the wrong team object type, but we can still load it:
  if (lines.at(0).compare(0, 5, "PKNTR") == 0)
  {
    ranked_neuralNet cRankedResult;
    // input actual team:
    if (!cRankedResult.input(lines, iLine)) { return false; }
    cResult = cRankedResult.getNetwork();
  }
  // this team object does not have any built-in ranking data, and should be easier to load:
  else if (lines.at(0).compare(0, 5, "PKNNT") == 0)
  {
    // input actual team:
    if (!cResult.input(lines, iLine)) { return false; }
  }
  else
  {
    std::cerr << "ERR " << __FILE__ << "." << __LINE__ << 
      ": File \"" << path << "\" has header of type \"" << lines.at(0).substr(0, 5) << 
      "\" (needs to be \"PKNTR\" or \"PKNNT\") and is incompatible with this program!\n";

    return false;
  }

  return true;
} // endOf input player team





bool PkIO::inputRankedTeam(const boost::filesystem::path& path, RankedTeam& cTeam)
{
  std::vector<std::string> lines;
  {
    std::string inputBuffer;

    if (loadFileToString(path, "PKAIR", inputBuffer) != true)
    {
      return false;
    }

    //tokenize by line endings
    lines = tokenize(inputBuffer, "\n\r");
  }

  size_t iLine = 0;
  return cTeam.input(lines, iLine);
} // endOf inputRankedTeam


TeamNonVolatile PkIO::inputPlayerTeam(const std::string& path) {
  TeamNonVolatile result;

  if (!inputPlayerTeam(path, result)) {
    throw std::runtime_error("Failed to load team");
  }

  return result;
}

bool PkIO::inputPlayerTeam(const boost::filesystem::path& path, TeamNonVolatile& cResult)
{
  size_t iLine = 0;
  std::vector<std::string> lines;
  {
    std::string inputBuffer;

    if (loadFileToString(path, "PKAI", inputBuffer) != true)
    {
      return false;
    }

    //tokenize by line endings
    lines = tokenize(inputBuffer, "\n\r");
  }

  // this is the wrong team object type, but we can still load it:
  if (lines.at(iLine).compare(0, 5, "PKAIR") == 0)
  {
    RankedTeam cTeam;
    // input actual team:
    if (!cTeam.input(lines, iLine)) { return false; }
    cResult = cTeam.team;
  }
  // this team object does not have any built-in ranking data, and should be easier to load:
  else if (lines.at(iLine).compare(0, 5, "PKAIE") == 0)
  {
    // input actual team:
    if (!cResult.input(lines, iLine)) { return false; }
  }
  else
  {
    std::cerr << "ERR " << __FILE__ << "." << __LINE__ << 
      ": File \"" << path << "\" has header of type \"" << lines.at(0).substr(0, 5) << 
      "\" (needs to be \"PKAIE\" or \"PKAIR\") and is incompatible with this program!\n";

    return false;
  }

  return true;
} // endOf input player team
