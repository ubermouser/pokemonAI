#include "../inc/trainer.h"

#include <algorithm>
#include <iomanip>
#include <iostream>
#include <math.h>

#include <boost/timer.hpp>
#include <boost/foreach.hpp>
#include <boost/scoped_ptr.hpp>

#include "../inc/pokedex.h"
#include "../inc/game.h"

#include "../inc/pokemon_nonvolatile.h"
#include "../inc/team_nonvolatile.h"

#include "../inc/planner_max.h"
#include "../inc/planner_stochastic.h"
#include "../inc/planner_directed.h"

#include "../inc/evaluator_featureVector.h"

#include "../inc/genetic.h"
#include "../inc/fp_compare.h"
#include "../inc/roulette.h"

template<class dataType>
size_t findInData(const std::vector<dataType>& data, uint64_t hash);

template<class data_t>
void shrinkDataPopulation(std::vector<data_t>& data, size_t targetSize);

Trainer::Trainer(
  uint32_t _gameType, 
  size_t _maxPlies, 
  size_t _maxMatches, 
  size_t _gameAccuracy,
  size_t _engineAccuracy,
  const trueSkillSettings& _tSettings,
  const experienceNetSettings& _expSettings,
  size_t _maxGenerations,
  fpType _workTime,
  fpType _mutationProbability,
  fpType _crossoverProbability,
  fpType _seedProbability,
  bool _enforceSameLeague,
  fpType _exploration,
  fpType _temperature,
  size_t _networkPopulations,
  const networkSettings_t& _netSettings,
  const std::vector<size_t>& _networkLayerSize,
  const std::array<size_t, 6>& _teamPopulations,
  fpType _seedNetworkProbability,
  size_t _jitterEpoch,
  size_t _numRollouts,
  size_t _writeoutInterval,
  const std::string& _teamPath,
  const std::string& _networkPath)
  : gameType(_gameType),
  tSettings(_tSettings),
  netSettings(_netSettings),
  expSettings(_expSettings),
  maxGenerations(_maxGenerations),
  teamPath(_teamPath),
  networkPath(_networkPath),
  writeOutEvery(_writeoutInterval),
  minimumWorkTime(_workTime),
  mutationProbability(_mutationProbability),
  crossoverProbability(_crossoverProbability),
  seedProbability(_seedProbability),
  seedNetworkProbability(_seedNetworkProbability),
  jitterEpoch(_jitterEpoch),
  numRollouts(_numRollouts),
  plannerAccuracy(_engineAccuracy),
  plannerExploration(_exploration),
  plannerTemperature(_temperature),
  teamPopulationSize(_teamPopulations),
  networkPopulationSize(_networkPopulations),
  networkLayerSize(_networkLayerSize),
  enforceSameLeague(_enforceSameLeague),
  generationsCompleted(0),
  heatsCompleted(),
  leagues(),
  networks(),
  evaluators(),
  trialTeam(NULL),
  trialNet(NULL),
  cGame(new Game(_maxPlies, _maxMatches, _gameAccuracy))
{
  ranked_neuralNet::initStatic(numRollouts);
  heatsCompleted.fill(0);
};





Trainer::~Trainer()
{
#ifdef _DISABLETEMPORALDIFFERENCE
  if (ranked_neuralNet::rolloutGame != NULL) { delete ranked_neuralNet::rolloutGame; ranked_neuralNet::rolloutGame = NULL; }
#endif

  ranked_neuralNet::uninitStatic();

  if (cGame != NULL) { delete cGame; }

  if (trialTeam != NULL) { delete trialTeam; }

  if (trialNet != NULL) { delete trialNet; } 
}





void Trainer::setGauntletTeam(const TeamNonVolatile& cTeam)
{
  bool gauntletModeSet = false;
  if (gameType == GT_OTHER_GAUNTLET_NET) 
  {
    gauntletModeSet = true;
    gameType = GT_OTHER_GAUNTLET_BOTH;
  }
  else if (gameType < GT_OTHER_GAUNTLET_TEAM)
  {
    gauntletModeSet = true;
    gameType = GT_OTHER_GAUNTLET_TEAM;
  }
  if (verbose >= 6 && gauntletModeSet)
  {
    std::cerr << "INF " << __FILE__ << "." << __LINE__ << 
      ": A gauntlet team was defined, but gauntlet mode was not selected. Auto-selecting gauntlet mode...\n";
  }

  if (trialTeam != NULL) { delete trialTeam; }
  trialTeam = new ranked_team(cTeam, 0, tSettings);
  trialTeam->generateHash();
}

void Trainer::setGauntletNetwork(const neuralNet& cNet)
{
  bool gauntletModeSet = false;
  if (gameType == GT_OTHER_GAUNTLET_TEAM) 
  {
    gauntletModeSet = true;
    gameType = GT_OTHER_GAUNTLET_BOTH;
  }
  else if (gameType < GT_OTHER_GAUNTLET_TEAM)
  {
    gauntletModeSet = true;
    gameType = GT_OTHER_GAUNTLET_NET;
  }
  if (verbose >= 6 && gauntletModeSet)
  {
    std::cerr << "INF " << __FILE__ << "." << __LINE__ << 
      ": A gauntlet network was defined, but gauntlet mode was not selected. Auto-selecting gauntlet mode...\n";
  }

  if (trialNet != NULL) { delete trialNet; }
  trialNet = new ranked_neuralNet(cNet, 0, netSettings, expSettings, tSettings);
  trialNet->generateHash();
}

bool Trainer::seedTeam(const TeamNonVolatile& cTeam)
{
  ranked_team cRankTeam(cTeam, 0, tSettings);
  cRankTeam.generateHash();
  if (isInPopulation(cRankTeam))
  {
    if (verbose >= 5)
    {
      std::cerr << "WAR " << __FILE__ << "." << __LINE__ << 
        ": Duplicate team \"" << cTeam.getName() << "\" was seeded, ignoring...\n";
    }
    return false; 
  }
  leagues[cTeam.getNumTeammates() -1].push_back(cRankTeam);
  return true;
}

bool Trainer::seedNetwork(const neuralNet& cNet)
{
  if (ranked_neuralNet::getEvaluator(cNet.numInputs(), cNet.numOutputs()) == NULL)
  {
    std::cerr << "ERR " << __FILE__ << "." << __LINE__ << 
      ": No loaded evaluators exist for given network topology!\n";
    return false;
  }

  ranked_neuralNet cRankNet(cNet, 0, netSettings, expSettings, tSettings);
  cRankNet.generateHash();

  if (isInNetworks(cRankNet))
  {
    if (verbose >= 5)
    {
      std::cerr << "WAR " << __FILE__ << "." << __LINE__ << 
        ": Duplicate network \"" << cNet.getName() << "\" was seeded, ignoring...\n";
    }
    return false; 
  }
  networks.push_back(cRankNet);
  return true;
}

bool Trainer::seedEvaluator(const Evaluator& eval)
{
  evaluators.push_back(ranked_evaluator(eval, 0, tSettings));
  return true;
}





bool Trainer::initialize()
{
  // make sure networks are correctly sized:
  if (networkLayerSize.size() < 2 || evaluator_featureVector::getEvaluator(networkLayerSize.front(), networkLayerSize.back()) == NULL)
  {
    std::cerr << "ERR " << __FILE__ << "." << __LINE__ << 
      ": No loaded evaluators exist for given network topology!\n";

    return false;
  }

  // no writeouts if there's no path to save a file
  if (teamPath.empty() && networkPath.empty() && writeOutEvery > 0) 
  { 
    if (verbose >= 5)
    {
      std::cerr << "WAR " << __FILE__ << "." << __LINE__ << 
        ": A writeout interval was defined without defining a backup location -- backups will be disabled.\n";
    }
    writeOutEvery = 0; 
  }

  if (!teamPath.empty())
  {
    if (!loadTeamPopulation()) { return false; }

    for (size_t iLeague = 0; iLeague != leagues.size(); ++iLeague)
    {
      std::vector<ranked_team>& cLeague = leagues[iLeague];
      size_t& leagueTargetSize = teamPopulationSize[iLeague];

      // don't bother operating on leagues which don't need it
      if (cLeague.size() == 0) { continue; }
      if (cLeague.size() <= leagueTargetSize) { continue; }

      std::sort(cLeague.begin(), cLeague.end());

      // TODO: should we add some other method to specify that we don't want the trainer to evolve that specific population?
      if (leagueTargetSize == 0) { continue; }

      // cull population with stored fitness values
      // TODO: should some assurance be made that the fitness values stored are relateable to eachother? A unique trainer tag?
      shrinkTeamPopulation(iLeague, leagueTargetSize);
    }
  }

  if (!networkPath.empty())
  {
    if (!loadNetworkPopulation()) { return false; }

    if ((networks.size() > 0) && (networks.size() >= networkPopulationSize) && (networkPopulationSize != 0))
    {
      // sort network for pruning:
      std::sort(networks.begin(), networks.end());

      // shrink its population to target:
      shrinkNetworkPopulation(networkPopulationSize);

    }
  }

  // reset rankings: (TODO: should be a switch for this
  /*{
    BOOST_FOREACH(std::vector<ranked_team>& cLeague, leagues)
    {
      BOOST_FOREACH(ranked& cRanked, cLeague)
      {
        cRanked.getSkill() = trueSkill(1, tSettings);
      }
    }
    BOOST_FOREACH(ranked& cRanked, networks)
    {
      cRanked.getSkill() = trueSkill(1, tSettings);
    }
  }*/

  return true;
}





void Trainer::evolve()
{
  bool printingNets, writingNets, printingTeams, writingTeams, spawnNets, spawnTeams;
  bool updateFirstNet = true;
  switch(gameType)
  {
  case GT_OTHER_GAUNTLET_NET:
    updateFirstNet = false;
  case GT_OTHER_EVONETS:
    printingTeams = false;
    printingNets = true;
    writingTeams = false;
    writingNets = !networkPath.empty();
    spawnNets = true;
    spawnTeams = false;
    break;
  case GT_OTHER_GAUNTLET_TEAM:
  case GT_OTHER_EVOTEAMS:
    printingTeams = true;
    printingNets = false;
    writingTeams = !teamPath.empty();
    writingNets = false;
    spawnNets = false;
    spawnTeams = true;
    break;
  case GT_OTHER_GAUNTLET_BOTH:
    updateFirstNet = false;
  default:
  case GT_OTHER_EVOBOTH:
    printingTeams = true;
    printingNets = true;
    writingTeams = !teamPath.empty();
    writingNets = !networkPath.empty();
    spawnNets = true;
    spawnTeams = true;
  }

  size_t iLeague;
  boost::timer workTimer;

  if (verbose >= 0)
  {
    std::cout << "Starting evolution process -- " << maxGenerations << " heats total!\n";
  }

  for (generationsCompleted = 0; generationsCompleted != maxGenerations; ++generationsCompleted)
  {
    // determine the league we intend to do useful work in:
    iLeague = determineWorkingLeague();

    if (iLeague == SIZE_MAX)
    {
      std::cerr << "ERR " << __FILE__ << "." << __LINE__ << 
        ": A working league could not be determined -- no leagues have any teams!\n";
      // don't write-out anything, just return
      return;
    }

    std::vector<ranked_team>& cLeague = leagues[iLeague];
    size_t& leagueTargetSize = teamPopulationSize[iLeague];

    // if we are to write our data out to disk this generation:
    if ((writeOutEvery > 0) && (generationsCompleted % writeOutEvery == 0) && (generationsCompleted > 0))
    {
      //write out population to disk
      if (writingTeams && !saveTeamPopulation()) { writeOutEvery = 0; }
      if (writingNets && !saveNetworkPopulation()) { writeOutEvery = 0; }
    }

    // perform mutation and crossover if this isn't the first generation: (in that case, evaluate first)
    size_t numTeamsMutated = 0, numTeamsCrossed = 0, numTeamsSeeded = 0;
    size_t numNetsMutated = 0, numNetsCrossed = 0, numNetsSeeded = 0;

    // if at some point we destroyed some of the teams in this league from the previous iteration, generate new ones:
    if (cLeague.size() < leagueTargetSize)
    {
      numTeamsSeeded += seedRandomTeamPopulation(iLeague, leagueTargetSize);
    }

    if (networks.size() < networkPopulationSize)
    {
      numNetsSeeded += seedRandomNetworkPopulation(networkPopulationSize);
    }

    if(heatsCompleted[iLeague] > 0)
    {
      if (spawnTeams)
      {
        spawnTeamChildren(iLeague, numTeamsMutated, numTeamsCrossed, numTeamsSeeded);
      };
    } // endOf create generation
    if (generationsCompleted > 0)
    {
      if (spawnNets)
      {
        spawnNetworkChildren(numNetsMutated, numNetsCrossed, numNetsSeeded);
      }
      // reset MSE accumulators:
      BOOST_FOREACH(ranked_neuralNet& cNet, networks) { cNet.resetMeanSquaredError(); }
    }

    
    PlannerMax sPlanner(plannerAccuracy);
    //planner_stochastic sPlanner(plannerAccuracy, plannerTemperature, plannerExploration);
    planner_directed dPlanner(experienceNet(1, expSettings), plannerAccuracy, plannerExploration);

    // perform a number of psuedoRandom games so long as we have time for them:
    // WARNING! THIS SHOULD BE TUNED SUCH THAT NEW TEAMS GET AT LEAST 5 GAMES IN BEFORE A POPULATION CULL!
    size_t numMatches = 0, lastMatches = 0;
    size_t numTeamUpdates = 0, numNetUpdates = 0;
    size_t numFailedMatches = 0;
    double aPlies = 0, aQuality = 0;
    workTimer.restart();
    boost::timer checkpointTimer;
    while(mostlyLT(workTimer.elapsed(),minimumWorkTime) && (numFailedMatches < MAXTRIES))
    {
      ranked_team* teamA = NULL;
      ranked* rankedA = NULL;
      // determine team_A based on criteria:
      switch(gameType)
      {
      case GT_OTHER_GAUNTLET_TEAM:
      case GT_OTHER_GAUNTLET_BOTH:
        teamA = trialTeam;
        break;
      default:
        // find via roulette a team that needs to be ranked better:
        {
          size_t iTeamA = roulette<ranked_team, sortByVariability>::select(cLeague, sortByVariability());
          if (iTeamA == SIZE_MAX) { break; } // unable to find team by variability!?
          // the "challenger", highest variability team needing a match:
          teamA = &cLeague[iTeamA];
        }
      }
      // determine net_A based on some criteria:
      switch(gameType)
      {
      case GT_OTHER_GAUNTLET_NET:
      case GT_OTHER_GAUNTLET_BOTH:
        rankedA = trialNet;
        break;
      default:
        // find via roulette a team that needs to be ranked better:
        {
          size_t iRankedA = findEvaluator();
          if (iRankedA < networks.size())
          {
            // the "challenger", highest variability team needing a match:
            rankedA = &networks[iRankedA];
          }
          else if ((iRankedA - networks.size()) < evaluators.size())
          {
            rankedA = &evaluators[iRankedA - networks.size()];
          }
          else
          {
            // COULD NOT FIND AN EVALUATOR
            break;
          }
        }
      }

      std::array<trueSkillTeam, 2> tsTeams;
      tsTeams[TEAM_A] = trueSkillTeam(*teamA, *rankedA);

      // find via roulette a team and network that would be a good match for this team:
      
      tsTeams[TEAM_B] = findMatch(tsTeams[TEAM_A]);

      // wasn't supposed to happen, try again // TODO: try a limited number of times
      if (tsTeams[TEAM_B].baseEvaluator == NULL || tsTeams[TEAM_B].baseTeam == NULL) { numFailedMatches++; continue; }

      // save match quality for later:
      fpType cMatchQuality = trueSkill::matchQuality(tsTeams[TEAM_A], tsTeams[TEAM_B], tSettings);

      // initialize game with these two teams:
      cGame->setTeam(TEAM_A, tsTeams[TEAM_A].baseTeam->team);
      cGame->setTeam(TEAM_B, tsTeams[TEAM_B].baseTeam->team);
      std::array<ranked_evaluator*, 2> evals = {{ dynamic_cast<ranked_evaluator*>(tsTeams[TEAM_A].baseEvaluator) , dynamic_cast<ranked_evaluator*>(tsTeams[TEAM_B].baseEvaluator) }};
      std::array<ranked_neuralNet*, 2> nets = {{ dynamic_cast<ranked_neuralNet*>(tsTeams[TEAM_A].baseEvaluator) , dynamic_cast<ranked_neuralNet*>(tsTeams[TEAM_B].baseEvaluator) }};
      bool successful = true;
      for (size_t iTeam = 0; iTeam != 2; ++iTeam)
      {
        // if we chose a neural network evaluator with a directed planner:
        if (nets[iTeam] != NULL) 
        {
          boost::scoped_ptr<evaluator_featureVector> cEval(nets[iTeam]->getEvaluator());
          if (cEval == NULL) { successful = false; break; }
          dPlanner.setEvaluator(*cEval);
          dPlanner.setExperience(nets[iTeam]->getExperience());

          cGame->setPlanner(iTeam, dPlanner);
        }
        // seed a seeded evaluator with a stochastic planner:
        else
        {
          assert(evals[iTeam] != NULL);
          sPlanner.setEvaluator(evals[iTeam]->getEvaluator());

          cGame->setPlanner(iTeam, sPlanner);
        }
      }

      // failed to initialize something, try again
      if (!successful) { break; }

      // initialize engine:
      if (!cGame->initialize()) { break; }

      // run game:
      cGame->run();

      // TODO: should we update component teams once and update the list? (if it takes too long)
      // determine component teams of teamA and teamB:
      findSubteams(tsTeams[TEAM_A], TEAM_A);
      findSubteams(tsTeams[TEAM_B], TEAM_B);

#ifdef _DISABLETEMPORALDIFFERENCE
      // clear stored rollout vars:
      ranked_neuralNet::rolloutFitnesses.clear();
#endif
      // update networks and teams:
      for (size_t iTeam = 0; iTeam != 2; ++iTeam)
      {
        if (nets[iTeam] != NULL)
        {
          numNetUpdates += nets[iTeam]->update(*cGame, tsTeams[iTeam], iTeam, iTeam==0?updateFirstNet:true);
        }
        else
        {
          numNetUpdates += evals[iTeam]->update(*cGame, tsTeams[iTeam], iTeam);
        }
        numTeamUpdates += tsTeams[iTeam].baseTeam->update(*cGame, tsTeams[iTeam], iTeam);
      }

      // update weights:
      BOOST_FOREACH(const GameResult& cGameResult, cGame->getGameResults())
      {
        trueSkill::update( tsTeams[TEAM_A], tsTeams[TEAM_B], cGameResult, tSettings);
      }

      aPlies += cGame->getResult().numPlies;
      aQuality += cMatchQuality;
      numMatches += cGame->getGameResults().size();
      numFailedMatches = 0;
      // print out useful data during every checkpoint:
      if (checkpointTimer.elapsed() >= 2.0)
      {
        double dTime = checkpointTimer.elapsed();
        double cMatches = (numMatches - lastMatches);
        if (verbose >= 0)
        {
          std::ostringstream preamble;
          preamble 
            << "Updt " << (iLeague + 1)
            << "." << (generationsCompleted + 1)
            << ":" << (numMatches);
          std::cout <<
            std::setw(18) << std::left << preamble.str()
            << "T=" << std::setw(7) << std::right << (minimumWorkTime - workTimer.elapsed())
            << " Updt/sec= " << std::setw(9) << (cMatches / dTime)
            << " aPly= " << std::setw(9) << (aPlies / cMatches)
            << " aQual= " << std::setw(9) << (aQuality / cMatches)
            << "\n";
          std::cout.flush();
        }
        lastMatches = numMatches;
        aPlies = 0; aQuality = 0;
        checkpointTimer.restart();
      }

      if (verbose >= 1)
      {
        // team A:
        if (printingTeams) { std::cout << "tA: " << *tsTeams[TEAM_A].baseTeam << "\n"; }
        if (printingNets)
        {
          if (nets[TEAM_A] != NULL) { std::cout << "nA: " << *nets[TEAM_A] << "\n"; }
          else { std::cout << "nA: " << *evals[TEAM_A] << "\n"; }
        }
        // team B:
        if (printingTeams) { std::cout << "tB: " << *tsTeams[TEAM_B].baseTeam << "\n"; }
        if (printingNets)
        {
          if (nets[TEAM_B] != NULL) { std::cout << "nB: " << *nets[TEAM_B] << "\n"; }
          else { std::cout << "nB: " << *evals[TEAM_B] << "\n"; }
        }
        std::cout.flush();
      }
    } // endOf perform matches

    if (numFailedMatches >= MAXTRIES)
    {
      std::cerr << "ERR " << __FILE__ << "." << __LINE__ << 
        ": No acceptable matches found in league \"" << iLeague << "\"!\n";
    }

    // set heat completed:
    heatsCompleted[iLeague] += 1;

    // free memory, we need it for genetic algorithm and statistics operations:
    cGame->cleanUp();

    // rehash the neural nets, as TD changes their weights
    if (spawnNets)
    {
      BOOST_FOREACH(ranked_neuralNet& cNet, networks)
      {
        // zero network's momentums:
        //cNet.bNet.zeroMomentums();

        // TD will have modified the neural network's weights if batchUpdate isn't enabled, so re-hash the network
        cNet.generateHash();
        cNet.defineName();
      }
    }

    if (verbose >= 0)
    {
      std::clog << "\n";
      if (printingTeams)
      {
        std::clog 
          << "TEAM: End of Heat " << std::setw(4) << (generationsCompleted+1)
          << " League " << std::setw(4) <<  (iLeague + 1)
          << ": nMatches=" << std::setw(4) << numMatches
          << " nUpd=" << std::setw(4) << numTeamUpdates
          << " nMut=" << std::setw(4) << numTeamsMutated
          << " nCros=" << std::setw(4) << numTeamsCrossed
          << " nSeed=" << std::setw(4) << numTeamsSeeded
          << "\n";
        std::clog.flush();
      }
      if (printingNets)
      {
        std::clog 
          << "NETS: End of Heat " << std::setw(4) << (generationsCompleted+1)
          << " League " << std::setw(4) <<  (iLeague + 1)
          << ": nMatches=" << std::setw(4) << numMatches
          << " nUpd=" << std::setw(4) << numNetUpdates
          << " nJit=" << std::setw(4) << numNetsMutated
          << " nCros=" << std::setw(4) << numNetsCrossed
          << " nSeed=" << std::setw(4) << numNetsSeeded
          << "\n";
        std::clog.flush();
      }
    }

    // sort the newly modified leaderboard:
    std::sort(cLeague.begin(), cLeague.end());
    std::sort(networks.begin(), networks.end());
    // calculate descriptive statistics of the leaderboard, print them to std::clog:
    if ((verbose >= 0) && ( generationsCompleted +1 ) != maxGenerations)
    {
      if (printingTeams)
      {
        teamTrainerResult cResult;
        calculateDescriptiveStatistics(iLeague, cResult);
        printLeagueStatistics(iLeague, 5, cResult);
      }
      if (printingNets)
      {
        networkTrainerResult cResult;
        calculateDescriptiveStatistics(cResult);
        printNetworkStatistics(5, cResult);
      }
    }

  } // endOf foreach generation

  // print out results:
  if (printingTeams)
  {
    size_t iLeague = 0;
    BOOST_FOREACH(std::vector<ranked_team>& cLeague, leagues)
    {
      if (cLeague.empty()) { iLeague++; continue; }
      if (teamPopulationSize[iLeague] == 0) { iLeague++; continue; }

      // assure the league is sorted
      std::sort(cLeague.begin(), cLeague.end());
      teamTrainerResult cResult;
      calculateDescriptiveStatistics(iLeague, cResult);
      printLeagueStatistics(iLeague, 15, cResult);
      iLeague++;
    }
  }
  if (printingNets)
  {
    if (!networks.empty() && networkPopulationSize > 0)
    {
      networkTrainerResult cResult;
      calculateDescriptiveStatistics(cResult);
      printNetworkStatistics(15, cResult);
    }
  }

  // writeOut the result at the end of the evolution period, if we're doing that:
  if (writeOutEvery > 0)
  {
    //write out population to disk
    if (writingTeams) { saveTeamPopulation(); }
    if (writingNets) { saveNetworkPopulation(); }
  }
}; // endOf Evolve





void Trainer::spawnTeamChildren(size_t iLeague, size_t& numMutated, size_t& numCrossed, size_t& numSeeded)
{
  std::vector<ranked_team> offspring;
  std::vector<ranked_team>& cLeague = leagues[iLeague];
  BOOST_FOREACH(ranked_team& cTeam, cLeague)
  {
    fpType cProbability = (double)rand()/(double)RAND_MAX;

    // if we should probabilistically cause a mutation to this team:
    if (cProbability < mutationProbability)
    {
      ranked_team mutatedTeam = ranked_team::mutate(tSettings, leagues, cTeam);

      if ((mutatedTeam.team.getNumTeammates() - 1) != iLeague) { continue; } // don't add a freak
      if (isInPopulation(mutatedTeam)) { continue; } // don't add a duplicate
      offspring.push_back(mutatedTeam);
      numMutated++;
    }
    // if we should probabilistically cause a crossover to this element and another psuedorandom element:
    else if ((1.0 - cProbability) < crossoverProbability)
    {
      // find the best crossover partner for cTeam that is not cTeam its self
      size_t iOTeam = roulette<ranked_team, sortByMean_noDuplicates>::select(cLeague, sortByMean_noDuplicates(cTeam, true));

      if (iOTeam == SIZE_MAX) { continue; }
      ranked_team& oTeam = cLeague[ iOTeam ];
        
      ranked_team crossedTeam = ranked_team::crossover(tSettings, cTeam, oTeam);

      if ((crossedTeam.team.getNumTeammates() - 1) != iLeague) { continue; } // don't add a freak
      if (isInPopulation(crossedTeam)) { continue; } // don't add a duplicate
      offspring.push_back(crossedTeam);
      numCrossed++;
    }
    // if we should probabilstically add a seed to the population:
    else if ((cProbability - mutationProbability) < seedProbability)
    {
      ranked_team seededTeam = ranked_team::selectRandom(tSettings, leagues, iLeague + 1);

      if ((seededTeam.team.getNumTeammates() - 1) != iLeague) { continue; } // don't add a freak
      if (isInPopulation(seededTeam)) { continue; } //  don't add a duplicate
      offspring.push_back(seededTeam);
      numSeeded++;
    }
  } // endOf mutation, crossover and seed loop

  // delete as many elements as necessary to allow the new offspring their place in the population:
  shrinkTeamPopulation(iLeague, teamPopulationSize[iLeague] - offspring.size());

  // add elements from offspring to population:
  for (size_t iOffspring = 0, iSize = offspring.size(); iOffspring != iSize; ++iOffspring)
  {
    cLeague.push_back(offspring[iOffspring]);
  }
  assert(cLeague.size() == teamPopulationSize[iLeague]);
};

void Trainer::spawnNetworkChildren(size_t& numJittered, size_t& numCrossed, size_t& numSeeded)
{
  std::vector<ranked_neuralNet> offspring;
  BOOST_FOREACH(ranked_neuralNet& cNet, networks)
  {
    fpType cProbability = (double)rand()/(double)RAND_MAX;

    // if we should probabilistically cause a mutation (jitter) to this network:
    /*if (cProbability < jitterNetworkProbability)
    {
      offspring.push_back(ranked_neuralNet::jitter_create(cNet, tSettings));

      numJittered++;
    }*/
    // if we should probabilstically add a seed to the population:
    if (cProbability < seedNetworkProbability)
    {
      offspring.push_back(ranked_neuralNet::generateRandom(networkLayerSize, netSettings, expSettings, tSettings));

      numSeeded++;
    }
  } // endOf mutation, crossover and seed loop

  // delete as many elements as necessary to allow the new offspring their place in the population:
  shrinkNetworkPopulation(networkPopulationSize - offspring.size());

  // force jitter networks which are stuck in local minima or have diverged:
  if (mostlyGT(netSettings.jitterMax, 0.0f))
  {
    BOOST_FOREACH(ranked_neuralNet& cNet, networks)
    {
      if  (
        mostlyGT(cNet.getMeanSquaredError(), 0.45)
        || 
        ((jitterEpoch > 0) && (cNet.gamesSinceJitter() >= jitterEpoch))
        )
      {
        cNet.jitter(tSettings); numJittered++;
      }
    }
  }

  // add elements from offspring to population:
  BOOST_FOREACH(ranked_neuralNet& offspringNet, offspring)
  {
    networks.push_back(offspringNet);
  }

  assert(networks.size() == networkPopulationSize);
};





size_t Trainer::seedRandomTeamPopulation(size_t iLeague, size_t targetSize)
{
  size_t numSeeded = 0;
  std::vector<ranked_team>& cLeague = leagues[iLeague];
  cLeague.reserve(targetSize);
  for (size_t iTeam = 0, iSize = targetSize - cLeague.size(); iTeam != iSize; ++iTeam)
  {
    bool isSuccessful = false;
    ranked_team cTeam;
    for (size_t numTries = 0; (numTries != MAXTRIES) && (!isSuccessful); ++numTries)
    {
      cTeam = ranked_team::selectRandom(tSettings, leagues, iLeague + 1);

      if (!isInPopulation(cTeam)){ isSuccessful = true; }
    }

    // don't bother trying anymore, we just can't generate any unique teams for some reason
    if (isSuccessful == false)
    {
      break;
    }

    cLeague.push_back(cTeam);
    numSeeded++;
  }

  // sort the league by skill after our new additions
  std::sort(cLeague.begin(), cLeague.end());
  return numSeeded;
}; //endOf seedRandomTeamPopulation

size_t Trainer::seedRandomNetworkPopulation(size_t targetSize)
{
  size_t numSeeded = 0;
  networks.reserve(targetSize);
  for (size_t iNetwork = 0, iSize = targetSize - networks.size(); iNetwork != iSize; ++iNetwork)
  {
    bool isSuccessful = false;
    ranked_neuralNet cNet;
    for (size_t numTries = 0; (numTries != MAXTRIES) && (!isSuccessful); ++numTries)
    {
      cNet = ranked_neuralNet::generateRandom(networkLayerSize, netSettings, expSettings, tSettings);

      if (!isInNetworks(cNet)){ isSuccessful = true; }
    }

    // don't bother trying anymore, we just can't generate any unique networks for some reason
    if (isSuccessful == false)
    {
      break;
    }

    networks.push_back(cNet);
    numSeeded++;
  }

  // sort the population by skill after our new additions
  std::sort(networks.begin(), networks.end());
  return numSeeded;
}; //endOf seedRandomNetworkPopulation




template<class data_t>
void shrinkDataPopulation(std::vector<data_t>& data, size_t targetSize)
{
  if (targetSize >= data.size()) { return; }
  if (targetSize == 0) { data.clear(); return; }

  // determine maximum mean:
  fpType maxMean = -std::numeric_limits<fpType>::infinity();
  BOOST_FOREACH(const data_t& cData, data)
  {
    fpType cMeanSkill = cData.getSkill().getMean();
    if (cMeanSkill > maxMean) { maxMean = cMeanSkill; }
  }

  // create an array of elements that we should delete via roulette selection:
  std::vector<size_t> toDelete = 
    roulette<data_t, sortByInverseMean>::selectDynamic(
    data, 
    (data.size() - targetSize), 
    sortByInverseMean(maxMean));

  // sort the array (so we don't delete an element and invalidate offests to elements above it:
  std::sort(toDelete.begin(), toDelete.end());

  // perform deletions:
  BOOST_REVERSE_FOREACH(size_t iCDelete, toDelete)
  {
    // couldn't find enough elements to delete!?
    if (iCDelete == SIZE_MAX) 
    { 
      assert(false && "could not find enough entities to delete!\n");
      break; 
    }

    // delete element
    data.erase(data.begin() + iCDelete);
  }
};

void Trainer::shrinkTeamPopulation(size_t iLeague, size_t targetSize)
{
  std::vector<ranked_team>& cLeague = leagues[iLeague];
  shrinkDataPopulation(cLeague, targetSize);
};

void Trainer::shrinkNetworkPopulation(size_t targetSize)
{
  shrinkDataPopulation(networks, targetSize);
};





size_t Trainer::determineWorkingLeague() const
{
  size_t iLeastHeats = SIZE_MAX;
  uint32_t leastHeats = UINT32_MAX;

  for (size_t iLeague = 0; iLeague != heatsCompleted.size(); ++iLeague)
  {
    // no work can be done if a population is empty
    if (teamPopulationSize[iLeague] == 0) { continue; }

    if (heatsCompleted[iLeague] < leastHeats)
    {
      leastHeats = heatsCompleted[iLeague];
      iLeastHeats = iLeague;
    }
  }

  return iLeastHeats;
} // endOf determineWorkingLeague




size_t Trainer::findEvaluator()
{
  std::vector<const ranked*> possibleEvaluators;
  possibleEvaluators.reserve(networks.size() + evaluators.size());
  BOOST_FOREACH(const ranked& cEvaluator, networks)
  {
    possibleEvaluators.push_back(&cEvaluator);
  }
  BOOST_FOREACH(const ranked& cEvaluator, evaluators)
  {
    possibleEvaluators.push_back(&cEvaluator);
  }

  return roulette<const ranked*, sortByVariability>::select(possibleEvaluators, sortByVariability());
}

trueSkillTeam Trainer::findMatch(const trueSkillTeam& oTeam)
{
  std::vector<trueSkillTeam> possibleMatches;
  std::vector<size_t> rankedSectionBegin;

  size_t iCLeague = oTeam.baseTeam->team.getNumTeammates() - 1;
  // feather by 1 around iCLeague if we're not enforcing the same league:
  size_t iEnd = enforceSameLeague?iCLeague:std::min((size_t)5U, (size_t)(iCLeague + 1U));
  size_t iBegin = enforceSameLeague?iCLeague:(size_t)(std::max(0, (int32_t)(iEnd - 2)));

  // find number of elements we're going to create:
  size_t numElements = 0;
  for (size_t iLeague = iBegin; iLeague != (iEnd + 1); ++iLeague)
  {
    numElements += leagues[iLeague].size();
  }
  numElements *= (networks.size() + (evaluators.size()));

  // reserve space for the array:
  possibleMatches.reserve(numElements);

  // add the two surrounding leagues as possible matches to this one, if enforce same league is not set:
  for (size_t iLeague = iBegin; iLeague != (iEnd + 1); ++iLeague)
  {
    rankedSectionBegin.push_back(possibleMatches.size());

    BOOST_FOREACH(ranked_team& cTeam, leagues[iLeague])
    {
      BOOST_FOREACH(ranked& cRanked, networks)
      {
        possibleMatches.push_back(trueSkillTeam(cTeam, cRanked));
      }
      BOOST_FOREACH(ranked& cRanked, evaluators)
      {
        possibleMatches.push_back(trueSkillTeam(cTeam, cRanked));
      }
    }
  }

  // find a match:
  size_t iResult = roulette<trueSkillTeam, sortByMatchQuality>::select(possibleMatches, sortByMatchQuality(oTeam, tSettings));
  if (iResult == SIZE_MAX) { return trueSkillTeam(); } 
  return possibleMatches[iResult];
} //endOf findMatch





template<class dataType>
size_t findInData(const std::vector<dataType>& data, uint64_t hash)
{
  // TODO: replace with hash table?
  size_t iData = 0;
  BOOST_FOREACH(const dataType& cData, data)
  {
    if (cData.compareHash(hash)) { return iData; }
    iData++;
  }
  return SIZE_MAX;
}

size_t Trainer::findInPopulation(size_t iLeague, uint64_t teamHash) const
{
  const std::vector<ranked_team>& cLeague = leagues[iLeague];
  return findInData(cLeague, teamHash);
}

bool Trainer::isInPopulation(const ranked_team& tTeam) const
{
#ifndef NDEBUG
  // assert team is valid:
  for (size_t iTeammate = 0, iSize = tTeam.team.getNumTeammates(); iTeammate != iSize; ++iTeammate)
  {
    assert(tTeam.team.isLegalSet(iTeammate, tTeam.team.teammate(iTeammate)));
  }
#endif
  return (findInPopulation(tTeam.team.getNumTeammates() -1 , tTeam.getHash()) != SIZE_MAX);
} //endOf isInPopulation

size_t Trainer::findInNetworks(uint64_t hash) const
{
  return findInData(networks, hash);
}

bool Trainer::isInNetworks(const ranked_neuralNet& cRankNet) const
{
  return findInNetworks(cRankNet.getHash()) != SIZE_MAX;
}





void Trainer::findSubteams(
  trueSkillTeam& cSTeam, 
  size_t iTeam)
{
  const ranked_team& cTeam = *cSTeam.baseTeam;

  // league:
  size_t initialLeague = cTeam.team.getNumTeammates() - 1;
  for (size_t iNLeague = 0; iNLeague != initialLeague; ++iNLeague)
  {
    size_t iLeague = initialLeague - iNLeague - 1;
    std::vector<ranked_team>& cLeague = leagues[iLeague];

    // exhaustively search hashes of teams for pokemon matches:
    BOOST_FOREACH(ranked_team& oTeam, cLeague)
    {
      bool isMatch = true;

      std::array<size_t, 6> correspondence;
      correspondence.fill(SIZE_MAX);
      std::array<bool, 6> isMatched;
      isMatched.fill(false);

      // foreach pokemon on current team:
      for (size_t iOTeammate = 0; iOTeammate != oTeam.team.getNumTeammates(); ++iOTeammate)
      {
        bool oTeammateMatch = false;

        // foreach pokemon in other team:
        for (size_t iCTeammate = 0; iCTeammate != cTeam.team.getNumTeammates(); ++iCTeammate)
        {
          // if previously matched, continue iterating:
          if (isMatched[iCTeammate]) { continue; }

          // if not a match, continue iterating:
          if ( cTeam.getTeammateHash(iCTeammate) != oTeam.getTeammateHash(iOTeammate) ) { continue; }

          oTeammateMatch = true;
          isMatched[iCTeammate] = true;
          correspondence[iOTeammate] = iCTeammate;
          break;
        } // endOf foreach otherTeammate

        // if one pokemon doesn't match, the whole team doesn't match
        if (!oTeammateMatch) { isMatch = false; break; }
      } // endOf foreach currentTeammate

      // go immediately to the next team if the current team is not a match:
      if (!isMatch) { continue; }

      cSTeam.push_back(oTeam, correspondence);

    } // endOf foreach team
  } // endOf foreach league lower than initialLeague

} // endOf findSubteams
