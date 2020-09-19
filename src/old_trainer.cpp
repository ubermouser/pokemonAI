#include "../inc/old_trainer.h"

#include <algorithm>
#include <iomanip>
#include <iostream>
#include <math.h>

#include <boost/timer.hpp>
#include <boost/foreach.hpp>

#include "../inc/game.h"

#include "../inc/pokemon_nonvolatile.h"
#include "../inc/team_nonvolatile.h"

#include "../inc/planner.h"

#include "../inc/genetic.h"
#include "../inc/fp_compare.h"
#include "../inc/roulette.h"

template<class dataType>
size_t findInData(const std::vector<dataType>& data, uint64_t hash);

template<class data_t>
void shrinkDataPopulation(std::vector<data_t>& data, size_t targetSize);

Trainer::Trainer(const Config& cfg) :
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
  trialTeam = new RankedTeam(cTeam, 0, tSettings);
  trialTeam->generateHash();
}

bool Trainer::seedTeam(const TeamNonVolatile& cTeam)
{
  RankedTeam cRankTeam(cTeam, 0, tSettings);
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

bool Trainer::seedEvaluator(const Evaluator& eval)
{
  evaluators.push_back(RankedEvaluator(eval, 0, tSettings));
  return true;
}





bool Trainer::initialize() {

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
      std::vector<RankedTeam>& cLeague = leagues[iLeague];
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

    std::vector<RankedTeam>& cLeague = leagues[iLeague];
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
      RankedTeam* teamA = NULL;
      Ranked* rankedA = NULL;
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
          size_t iTeamA = roulette<RankedTeam, sortByVariability>::select(cLeague, sortByVariability());
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

      std::array<TrueSkillTeam, 2> tsTeams;
      tsTeams[TEAM_A] = TrueSkillTeam(*teamA, *rankedA);

      // find via roulette a team and network that would be a good match for this team:
      
      tsTeams[TEAM_B] = findMatch(tsTeams[TEAM_A]);

      // wasn't supposed to happen, try again // TODO: try a limited number of times
      if (tsTeams[TEAM_B].baseEvaluator == NULL || tsTeams[TEAM_B].baseTeam == NULL) { numFailedMatches++; continue; }

      // save match quality for later:
      fpType cMatchQuality = TrueSkill::matchQuality(tsTeams[TEAM_A], tsTeams[TEAM_B], tSettings);

      // initialize game with these two teams:
      cGame->setTeam(TEAM_A, tsTeams[TEAM_A].baseTeam->team);
      cGame->setTeam(TEAM_B, tsTeams[TEAM_B].baseTeam->team);
      std::array<RankedEvaluator*, 2> evals = {{ dynamic_cast<RankedEvaluator*>(tsTeams[TEAM_A].baseEvaluator) , dynamic_cast<RankedEvaluator*>(tsTeams[TEAM_B].baseEvaluator) }};
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
        TrueSkill::update( tsTeams[TEAM_A], tsTeams[TEAM_B], cGameResult, tSettings);
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
        TrainerResult cResult;
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
    BOOST_FOREACH(std::vector<RankedTeam>& cLeague, leagues)
    {
      if (cLeague.empty()) { iLeague++; continue; }
      if (teamPopulationSize[iLeague] == 0) { iLeague++; continue; }

      // assure the league is sorted
      std::sort(cLeague.begin(), cLeague.end());
      TrainerResult cResult;
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
  std::vector<RankedTeam> offspring;
  std::vector<RankedTeam>& cLeague = leagues[iLeague];
  BOOST_FOREACH(RankedTeam& cTeam, cLeague)
  {
    fpType cProbability = (double)rand()/(double)RAND_MAX;

    // if we should probabilistically cause a mutation to this team:
    if (cProbability < mutationProbability)
    {
      RankedTeam mutatedTeam = RankedTeam::mutate(tSettings, leagues, cTeam);

      if ((mutatedTeam.team.getNumTeammates() - 1) != iLeague) { continue; } // don't add a freak
      if (isInPopulation(mutatedTeam)) { continue; } // don't add a duplicate
      offspring.push_back(mutatedTeam);
      numMutated++;
    }
    // if we should probabilistically cause a crossover to this element and another psuedorandom element:
    else if ((1.0 - cProbability) < crossoverProbability)
    {
      // find the best crossover partner for cTeam that is not cTeam its self
      size_t iOTeam = roulette<RankedTeam, sortByMean_noDuplicates>::select(cLeague, sortByMean_noDuplicates(cTeam, true));

      if (iOTeam == SIZE_MAX) { continue; }
      RankedTeam& oTeam = cLeague[ iOTeam ];
        
      RankedTeam crossedTeam = RankedTeam::crossover(tSettings, cTeam, oTeam);

      if ((crossedTeam.team.getNumTeammates() - 1) != iLeague) { continue; } // don't add a freak
      if (isInPopulation(crossedTeam)) { continue; } // don't add a duplicate
      offspring.push_back(crossedTeam);
      numCrossed++;
    }
    // if we should probabilstically add a seed to the population:
    else if ((cProbability - mutationProbability) < seedProbability)
    {
      RankedTeam seededTeam = RankedTeam::selectRandom(tSettings, leagues, iLeague + 1);

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





size_t Trainer::seedRandomTeamPopulation(size_t iLeague, size_t targetSize)
{
  size_t numSeeded = 0;
  std::vector<RankedTeam>& cLeague = leagues[iLeague];
  cLeague.reserve(targetSize);
  for (size_t iTeam = 0, iSize = targetSize - cLeague.size(); iTeam != iSize; ++iTeam)
  {
    bool isSuccessful = false;
    RankedTeam cTeam;
    for (size_t numTries = 0; (numTries != MAXTRIES) && (!isSuccessful); ++numTries)
    {
      cTeam = RankedTeam::selectRandom(tSettings, leagues, iLeague + 1);

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
  std::vector<RankedTeam>& cLeague = leagues[iLeague];
  shrinkDataPopulation(cLeague, targetSize);
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
  std::vector<const Ranked*> possibleEvaluators;
  possibleEvaluators.reserve(networks.size() + evaluators.size());
  BOOST_FOREACH(const Ranked& cEvaluator, networks)
  {
    possibleEvaluators.push_back(&cEvaluator);
  }
  BOOST_FOREACH(const Ranked& cEvaluator, evaluators)
  {
    possibleEvaluators.push_back(&cEvaluator);
  }

  return roulette<const Ranked*, sortByVariability>::select(possibleEvaluators, sortByVariability());
}

TrueSkillTeam Trainer::findMatch(const TrueSkillTeam& oTeam)
{
  std::vector<TrueSkillTeam> possibleMatches;
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

    BOOST_FOREACH(RankedTeam& cTeam, leagues[iLeague])
    {
      BOOST_FOREACH(Ranked& cRanked, networks)
      {
        possibleMatches.push_back(TrueSkillTeam(cTeam, cRanked));
      }
      BOOST_FOREACH(Ranked& cRanked, evaluators)
      {
        possibleMatches.push_back(TrueSkillTeam(cTeam, cRanked));
      }
    }
  }

  // find a match:
  size_t iResult = roulette<TrueSkillTeam, sortByMatchQuality>::select(possibleMatches, sortByMatchQuality(oTeam, tSettings));
  if (iResult == SIZE_MAX) { return TrueSkillTeam(); }
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
  const std::vector<RankedTeam>& cLeague = leagues[iLeague];
  return findInData(cLeague, teamHash);
}

bool Trainer::isInPopulation(const RankedTeam& tTeam) const
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





void Trainer::findSubteams(
  TrueSkillTeam& cSTeam,
  size_t iTeam)
{
  const RankedTeam& cTeam = *cSTeam.baseTeam;

  // league:
  size_t initialLeague = cTeam.team.getNumTeammates() - 1;
  for (size_t iNLeague = 0; iNLeague != initialLeague; ++iNLeague)
  {
    size_t iLeague = initialLeague - iNLeague - 1;
    std::vector<RankedTeam>& cLeague = leagues[iLeague];

    // exhaustively search hashes of teams for pokemon matches:
    BOOST_FOREACH(RankedTeam& oTeam, cLeague)
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
