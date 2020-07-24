
#include "../inc/ranked_neuralNet.h"

#include <map>
#include <algorithm>
#include <math.h>

#include <boost/array.hpp>
#include <boost/foreach.hpp>
#include <boost/scoped_ptr.hpp>

#include "../inc/environment_nonvolatile.h"
#include "../inc/environment_volatile.h"

#include "../inc/planner_directed.h"

#include "../inc/evaluator_featureVector.h"

#include "../inc/signature.h"
#include "../inc/fp_compare.h"
#include "../inc/init_toolbox.h"

#include "../inc/planner_random.h"
#include "../inc/game.h"

const std::string ranked_neuralNet::header = "PKNTR1";

void ranked_neuralNet::initStatic(size_t numRollouts)
{
  evaluator_featureVector::initStatic();

  // rollouts:
#ifdef _DISABLETEMPORALDIFFERENCE
  rolloutGame = new game(MAXPLIES, numRollouts, 1, true);
#endif
};

void ranked_neuralNet::uninitStatic()
{
  evaluator_featureVector::uninitStatic();

#ifdef _DISABLETEMPORALDIFFERENCE
  if (rolloutGame != NULL) { delete rolloutGame; }
#endif
}


ranked_neuralNet::ranked_neuralNet(
  const neuralNet& nNet, 
  size_t generation, 
  const networkSettings_t& cSettings,
  const experienceNetSettings& eSettings,
  const trueSkillSettings& settings)
  : ranked(generation, settings),
  bNet(nNet, cSettings),
  experience(nNet.isInitialized()?nNet.numInputs():1, eSettings),
  hash(ranked::defaultHash),
  totalMeanSquaredError(0.0),
  numUpdates(0.0),
  lastJitterEpoch(0)
{
};

fpType ranked_neuralNet::getMeanSquaredError() const
{
  if (mostlyEQ(numUpdates, 0.0)) { return 0.0; }
  return totalMeanSquaredError / numUpdates;
};

void ranked_neuralNet::setNetwork(const neuralNet& cNet)
{
  if (cNet.isInitialized()) { experience.resize(cNet.numInputs()); }

  bNet = network_t(cNet);

  resetMeanSquaredError();

  // re-hash network:
  generateHash();

  // create name:
  defineName();

  // ensure not saved:
  stateSaved = false;
}

ranked_neuralNet ranked_neuralNet::generateRandom(
  const std::vector<size_t>& layerWidths, 
  const networkSettings_t& cSettings,
  const experienceNetSettings& eSettings,
  const trueSkillSettings& tSettings)
{	
  ranked_neuralNet randomNet(neuralNet(layerWidths.begin(), layerWidths.end()), 0, cSettings, eSettings, tSettings);
  randomNet.bNet.randomizeWeights();

  // hash network:
  randomNet.generateHash();

  // create name:
  randomNet.defineName();

  return randomNet;
};

ranked_neuralNet ranked_neuralNet::jitter_create(const ranked_neuralNet& parent, const trueSkillSettings& settings)
{
  ranked_neuralNet jitteredNet(parent);

  jitteredNet.jitter(settings);

  // ensure record is zeroed
  jitteredNet.resetRecord();

  return jitteredNet;
};

void ranked_neuralNet::jitter(const trueSkillSettings& settings)
{
  // jitter network:
  bNet.jitterNetwork();

  // feather trueskill:
  getSkill().feather(settings);
  
  // increment generation
  generation++;

  // set that the network was jittered:
  lastJitterEpoch = getNumGamesPlayed();

  // re-hash network:
  generateHash();

  // create name:
  defineName();

  // ensure not saved:
  stateSaved = false;
}

size_t ranked_neuralNet::update(const game& cGame, const trueSkillTeam& cTeam, size_t iTeam, bool updateWeights)
{
  // update ranked vars:
  ranked::update(cGame, cTeam, iTeam);

  if (updateWeights)
  {
    // update ranked_neuralNet specific vars:
    for (size_t iGame = 0, iSize = cGame.getGameResults().size(); iGame != iSize; ++iGame)
    {
      propagate(cGame.getGameLog(iGame), cGame.getEnvNV(), *this, iTeam);
    }
  }

  /*if(updateExperience)
  {
    for (size_t iGame = 0, iSize = cGame.getGameResults().size(); iGame != iSize; ++iGame)
    {
      updateExperience(cGame.getGameLog(iGame), cGame.getEnvNV(), *this, iTeam);
    }
  }
  else */ // store experience from evaluator:
  {
    const planner_directed* cPlanner = dynamic_cast<const planner_directed*>(cGame.getPlanner(iTeam));
    if (cPlanner != NULL) { experience = cPlanner->getExperience(); }
    else
    {
      for (size_t iGame = 0, iSize = cGame.getGameResults().size(); iGame != iSize; ++iGame)
      {
        updateExperience(cGame.getGameLog(iGame), cGame.getEnvNV(), *this, iTeam);
      }
    }
  }

  return cGame.getGameResults().size();
}; // endOf update

float discreteClassifier(float output, uint32_t desired) 
{
  if ((output > 0.5f) && (desired > 0)) { return output; }
  else if ((output <= 0.5f) && (desired == 0)) { return output; }
  else { return ((desired>0)?1.0f:0.0f); }
};

float continuousClassifier(float desired) 
{
  desired = std::max(0.0f, std::min(1.0f, deScale(desired, 0.85f, 0.15f)));
  return desired;
};


#ifdef _DISABLETEMPORALDIFFERENCE
game* ranked_neuralNet::rolloutGame = NULL;
std::vector<float> ranked_neuralNet::rolloutFitnesses;

void ranked_neuralNet::propagate(
    const std::vector<turn>& turns, 
    const environment_nonvolatile& envNV,
    ranked_neuralNet& rankedNet, 
    size_t agentTeam)
{
  network_t& bNet = rankedNet.bNet;
  if (mostlyEQ(bNet.getSettings().learningRate, 0.0f)) { return; } // don't perform learning if learnrate is zero
  //std::vector<float> desiredFitnesses;
  boost::array<float, EVAL_OUTPUTNEURONS> resultVector;

  netEvaluator->resetEvaluator(envNV);

  // if we haven't cached fitnesses from the last game we played, generate new ones:
  if (rolloutFitnesses.empty())
  {
    rolloutGame->setPlanner(TEAM_A, planner_random());
    rolloutGame->setPlanner(TEAM_B, planner_random());

    rolloutGame->setEnvironment(envNV);

    rolloutFitnesses.reserve(turns.size());
    BOOST_FOREACH(const turn& cTurn, turns)
    {
      float desiredFitness;
      if (&cTurn != &turns.back())
      {
        // calculate fitness via rollout:
        if (!rolloutGame->initialize()) { return; }
        rolloutGame->setInitialState(cTurn.env);
        rolloutGame->run();

        const heatResult& cRolloutResult = rolloutGame->getResult();
        desiredFitness = ((float)cRolloutResult.score[TEAM_A]) / (float)(cRolloutResult.score[TEAM_A]+cRolloutResult.score[TEAM_B]);
      }
      else
      {
        // take terminal fitness:
        desiredFitness = (float) cTurn.depthMaxFitness[TEAM_A];
      }

      rolloutFitnesses.push_back(desiredFitness);
    }

    // if we're printing a feature descriptor to an ostream:
    if (featureStream != NULL)
    {
      bNet.getNeuralNet().clearInput();
      for (size_t iTurn = 0; iTurn != turns.size(); ++iTurn)
      {
        netEvaluator->seed(bNet.getNeuralNet(), turns[iTurn].env, TEAM_A);
        for (size_t iInput = 0; iInput < bNet.getNeuralNet().numInputs(); ++iInput)
        {
          *featureStream << *(bNet.getNeuralNet().inputBegin() + iInput) << ", ";
        }
        // print output:
        *featureStream << (rolloutFitnesses[iTurn]);
#if (EVAL_OUTPUTNEURONS == 2)
        *featureStream << ", " << (1.0f - rolloutFitnesses[iTurn]);
#endif
        *featureStream << "\n";
      }
      featureStream->flush();
    }
  }
  assert(rolloutFitnesses.size() == turns.size());

  // foreach side: (each network is trained for both sides)
  size_t _iTeam = rand()%2; // randomize first team trained to remove order effect
  for (size_t iNTeam = 0; iNTeam < 2; ++iNTeam)
  {
    size_t iTeam = (iNTeam+_iTeam)%2;

    // determine an order that we intend to visit the turns:
    std::vector<size_t> order(turns.size());
    for (size_t iTurn = 0; iTurn != turns.size(); ++iTurn) { order[iTurn] = iTurn; }
    std::random_shuffle(order.begin(), order.end()); // shuffle

    bNet.getNeuralNet().clearInput();

    // foreach turn:
    for (size_t iNTurn = 0; iNTurn != turns.size(); ++iNTurn)
    {
      size_t iTurn = order[iNTurn];

      const turn& cTurn = turns[iTurn];
      const environment_volatile& cEnv = cTurn.env;

      // feed forward with cEnv and envNV feature vector, but do not interpret output:
      netEvaluator->seed(bNet.getNeuralNet(), cEnv, iTeam);
      bNet.getNeuralNet().feedForward();

      // backpropagate a terminal result:
      constFloatIterator_t cOutValue = bNet.getNeuralNet().outputBegin();
      boost::array<float, EVAL_OUTPUTNEURONS>::iterator cRValue = resultVector.begin();
      // find aggregate variables:
      // target fitness is the fitness (at maximum depth) returned by the next ply:
      float desiredFitness = rolloutFitnesses[iTurn];
      if (iTeam != TEAM_A) { desiredFitness = 1.0f - desiredFitness; }

      // fitnessA:
      float cOutput = continuousClassifier(desiredFitness);
      *cRValue++ = (float) cOutput;
      /*// fitnessB:
      cOutput = continuousClassifier(1.0f - desiredFitness);
      *cRValue++ = (float) cOutput;*/

      rankedNet.totalMeanSquaredError += bNet.backPropagate(resultVector.begin());
      // perform update
      rankedNet.updateWeights();
      rankedNet.numUpdates += (fpType)bNet.getNeuralNet().numOutputs();
    } // endOf foreach turn

    /*if ((iNTeam+1) % 20 == 0)
    {
      float _MSE = rankedNet.totalMeanSquaredError/rankedNet.numUpdates;
      std::cout << std::setw(4) << iNTeam << " " << _MSE << "\n";
    }*/

  } // endOf foreach team
}; // endOf update_TD



#elif defined(_DISABLETEMPORALTRACE)



void ranked_neuralNet::propagate(
    const std::vector<turn>& turns, 
    const environment_nonvolatile& envNV,
    ranked_neuralNet& rankedNet, 
    size_t agentTeam)
{
  network_t& bNet = rankedNet.bNet;
  if (mostlyEQ(bNet.getSettings().learningRate, 0.0f)) { return; } // don't perform learning if learnrate is zero
  //boost::array<float, evaluator_network_t::numOutputNeurons> resultVector;
  std::vector<float> resultVector(bNet.getNeuralNet().numOutputs(), 0.0f);
  
  boost::scoped_ptr<evaluator_featureVector> netEvaluator(rankedNet.getEvaluator());
  if (netEvaluator == NULL) { return; }
  netEvaluator->resetEvaluator(envNV);

  // foreach side: (each network is trained for both sides)
  size_t _iTeam = rand()%2; // randomize first team trained to remove order effect
  for (size_t iNTeam = 0; iNTeam < 2; ++iNTeam)
  {
    size_t iTeam = (iNTeam+_iTeam)%2;

    // determine an order that we intend to visit the turns:
    std::vector<size_t> order(turns.size());
    for (size_t iTurn = 0; iTurn != turns.size(); ++iTurn) { order[iTurn] = turns.size() - iTurn - 1; } // backward
    //for (size_t iTurn = 0; iTurn != turns.size(); ++iTurn) { order[iTurn] = iTurn; } // forward

    bNet.getNeuralNet().clearInput();

    // foreach turn:
    for (size_t iNTurn = 0; iNTurn != turns.size(); ++iNTurn)
    {
      size_t iTurn = order[iNTurn];

      const turn& cTurn = turns[iTurn];
      const environment_volatile& cEnv = cTurn.env;

      // terminal turn first:
      //if ((iNTurn + 1) == turns.size()) // forward
      if (iNTurn == 0) // backward
      {
        // feed forward with cEnv and envNV feature vector, but do not interpret output:
        netEvaluator->seed(bNet.getNeuralNet(), cEnv, iTeam);
        bNet.getNeuralNet().feedForward();

        // backpropagate a terminal result:
        //constFloatIterator_t cOutValue = bNet.getNeuralNet().outputBegin();
        floatIterator_t cRValue = resultVector.begin();
        // find aggregate variables:
        // target fitness is the fitness (at maximum depth) returned by the next ply:
        float desiredFitness = (float) cTurn.depthMaxFitness[iTeam];

        // fitnessA:
        float cOutput = continuousClassifier(desiredFitness);
        cRValue[0] = (float) cOutput;
#if (EVAL_OUTPUTNEURONS >= 2)
        // fitnessB:
        cOutput = continuousClassifier(1.0f - desiredFitness);
        cRValue[1] = (float) cOutput;
#endif

        rankedNet.totalMeanSquaredError += bNet.backPropagate(resultVector.begin());
        // perform update
        rankedNet.updateWeights();
        rankedNet.numUpdates += (fpType)bNet.getNeuralNet().numOutputs();
      }
      // non-terminal turn:
      else if ((iNTurn + 1) != turns.size()) // backward
      //else if (iNTurn > 0) // forward
      {
        // feed forward with cEnv and envNV feature vector, but do not interpret output:
        netEvaluator->seed(bNet.getNeuralNet(), cEnv, iTeam);
        bNet.getNeuralNet().feedForward();

        // save current turn output:
        constFloatIterator_t cOutValue = bNet.getNeuralNet().outputBegin();
        BOOST_FOREACH(float& cOutput, resultVector)
        {
          cOutput = *cOutValue++;
        }

        // calculate previous node weights and output:
        const turn& pTurn = turns[iTurn-1];
        // determine previous node output
        netEvaluator->seed(bNet.getNeuralNet(), pTurn.env, iTeam);
        bNet.getNeuralNet().feedForward();

        // propagate the current evaluation as the previous evaluation's error:
        rankedNet.totalMeanSquaredError += bNet.backPropagate(resultVector.begin());
        // perform update
        rankedNet.updateWeights();
        rankedNet.numUpdates += (fpType)bNet.getNeuralNet().numOutputs();
      }
      /*else // iNTurn = 0 .. forward case only
      {
        // determine previous node output
        netEvaluator->seed(bNet.getNeuralNet(), cEnv, iTeam);
        bNet.getNeuralNet().feedForward();
        // update eligibility traces based on lambda
        bNet.updateEligibilities();
      }*/
    } // endOf foreach turn
  } // endOf foreach team
}; // endOf update_TD
#endif





void ranked_neuralNet::updateExperience(
  const std::vector<turn>& turns, 
  const environment_nonvolatile& envNV,
  ranked_neuralNet& rankedNet, 
  size_t agentTeam)
{
  experienceNet& cExperience = rankedNet.experience;
  std::vector<float> resultVector(rankedNet.bNet.getNeuralNet().numInputs(), 0.0f);

  boost::scoped_ptr<evaluator_featureVector> netEvaluator(rankedNet.getEvaluator());
  if (netEvaluator == NULL) { return; }
  netEvaluator->resetEvaluator(envNV);

  BOOST_FOREACH(const turn& cTurn, turns)
  {
    // seed to buffer:
    netEvaluator->seed(resultVector.begin(), cTurn.env, agentTeam);

    // and load buffer into experience:
    cExperience.addExperience(resultVector.begin());
  };
};




void ranked_neuralNet::updateWeights()
{
  bNet.updateWeights();
};





void ranked_neuralNet::generateHash()
{
  // output what we would be printing to the file to a stringstream:
  std::stringstream netVal;
  bNet.getNeuralNet().output(netVal, false);

  // hash the resulting character array:
  hash = hashes::hash_murmur3(netVal.str().c_str(), netVal.str().length());
}

void ranked_neuralNet::defineName()
{
  // set name of created network based on hash:
  std::ostringstream tName(std::ostringstream::out);
  tName << "NT-x" << std::setfill('0') << std::setw(16) << std::hex << getHash();
  bNet.getNeuralNet().setName(tName.str());
}




evaluator_featureVector* ranked_neuralNet::getEvaluator(const neuralNet& cNet)
{
  return evaluator_featureVector::getEvaluator(cNet);
};

const evaluator_featureVector* ranked_neuralNet::getEvaluator(size_t numInputNeurons, size_t numOutputNeurons)
{
  return evaluator_featureVector::getEvaluator(numInputNeurons, numOutputNeurons);
};

evaluator_featureVector* ranked_neuralNet::getEvaluator() const
{
  return getEvaluator(bNet.getNeuralNet());
};





bool ranked_neuralNet::hasEvaluator() const
{
  return hasEvaluator(bNet.getNeuralNet());
};

bool ranked_neuralNet::hasEvaluator(const neuralNet& cNet)
{
  return evaluator_featureVector::hasEvaluator(cNet);
};





std::ostream& operator <<(std::ostream& os, const ranked_neuralNet& tR)
{
  os << 
    "" << std::setw(20) << tR.bNet.getNeuralNet().getName().substr(0,20) << " ";

  // print a few characters from each layer:
  std::ostringstream tLayers(std::ostringstream::out);
  for (size_t iLayer = 0; iLayer != tR.bNet.getNeuralNet().getNumLayers(); ++iLayer)
  {
    tLayers << (tR.bNet.getNeuralNet().getWidth(iLayer) / 16);
    if ((iLayer+1) != tR.bNet.getNeuralNet().getNumLayers()) { tLayers << "-"; }
  }
  size_t prevPrecision = os.precision();
  //size_t flags = os.flags();
  os.precision(5);
  //os.unsetf(std::ios::floatfield);
  //os.setf(std::ios::scientific);
  os <<
    std::setw(10) << std::left << tLayers.str().substr(0, 10) <<
    " e= " << std::setw(10) << std::right << tR.getMeanSquaredError();
  //os.setf(flags);
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





void ranked_neuralNet::output(std::ostream& oFile, bool printHeader) const
{
  // header:
  if (printHeader)
  {
    oFile << header;
  };
  oFile << "\t" << std::hex << getHash() << std::dec << "\n";

  // out put ranked data:
  ranked::output(oFile);

  // output neural network:
  bNet.getNeuralNet().output(oFile);
};

bool ranked_neuralNet::input(const std::vector<std::string>& lines, size_t& iLine)
{
  // are the enough lines in the input stream:
  if ((lines.size() - iLine) < 1U)
  {
    std::cerr << "ERR " << __FILE__ << "." << __LINE__ << 
      ": unexpected end of input stream at line " << iLine << "!\n";
    return false; 
  }

  // compare neuralNetwork_ranked header:
  if (lines.at(iLine).compare(0, header.size(), header) != 0)
  {
    std::cerr << "ERR " << __FILE__ << "." << __LINE__ << 
      ": ranked_neuralNet stream has header of type \"" << lines.at(iLine).substr(0, header.size()) << 
      "\" (needs to be \"" << header << 
      "\") and is incompatible with this program!\n";

    return false;
  }

  // input hash:
  {
    std::vector<std::string> tokens = INI::tokenize(lines.at(iLine), "\t");
    if (!INI::checkRangeB(tokens.size(), (size_t)2, (size_t)2)) { return false; }

    std::istringstream tokenStream(tokens.at(1), std::istringstream::in);

    if (!(tokenStream >> std::hex >> hash)) { INI::incorrectArgs("ranked_neuralNet hash", iLine, 0); return false; }
  }

  iLine++;
  // input ranked object:
  if (!ranked::input(lines, iLine)) { return false; }

  // input neural network:
  {
    neuralNet cNet;
    if (!cNet.input(lines, iLine)) { return false; }
    bNet = network_t(cNet, bNet.getSettings());
  }

  // don't jitter right away:
  lastJitterEpoch = getNumGamesPlayed();

  // make sure experienceNet is of the right size:
  experience.resize(bNet.getNeuralNet().numInputs());

  return true;
};
