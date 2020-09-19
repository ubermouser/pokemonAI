#include "../inc/trainer_neural.h"

#include "../inc/evaluator_featureVector.h"
#include "inc/neural_trainer.h"


void TrainerNeural::initialize() {
  base_t::initialize();

  // make sure networks are correctly sized:
  if (networkLayerSize.size() < 2 || evaluator_featureVector::getEvaluator(networkLayerSize.front(), networkLayerSize.back()) == NULL)
  {
    std::cerr << "ERR " << __FILE__ << "." << __LINE__ <<
      ": No loaded evaluators exist for given network topology!\n";

    return false;
  }

  if (!networkPath.empty()) {
    if (!loadNetworkPopulation()) { return false; }

    if ((networks.size() > 0) && (networks.size() >= networkPopulationSize) && (networkPopulationSize != 0))
    {
      // sort network for pruning:
      std::sort(networks.begin(), networks.end());

      // shrink its population to target:
      shrinkNetworkPopulation(networkPopulationSize);

    }
  }
}


void Trainer::setGauntletNetwork(const neuralNet& cNet) {
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


bool TrainerNeural::seedNetwork(const neuralNet& cNet) {
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


void TrainerNeural::shrinkNetworkPopulation(size_t targetSize)
{
  shrinkDataPopulation(networks, targetSize);
};


size_t TrainerNeural::seedRandomNetworkPopulation(size_t targetSize) {
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


void TrainerNeural::spawnNetworkChildren(size_t& numJittered, size_t& numCrossed, size_t& numSeeded)
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


size_t Trainer::findInNetworks(uint64_t hash) const
{
  return findInData(networks, hash);
}

bool Trainer::isInNetworks(const ranked_neuralNet& cRankNet) const
{
  return findInNetworks(cRankNet.getHash()) != SIZE_MAX;
}

bool TrainerNeural::loadNetworkPopulation()
{
  // assure path exists:
  boost::filesystem::path pPath(networkPath);

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
    std::cout << "Loading backup population from disk at location \"" << networkPath << "\"...\n";
  }

  // load all elements in directory:
  size_t numLoaded = 0;
  size_t numTotal = 0;
  size_t numIncorrectHashes = 0;
  for ( boost::filesystem::directory_iterator cNetworkFile(pPath), endNetworkFile; cNetworkFile != endNetworkFile; ++cNetworkFile)
  {
    bool isSuccessful = false;
    // ignore directories
    if (boost::filesystem::is_directory(*cNetworkFile)) { continue; }

    // make sure extension is txt, ignore all others:
    if ((cNetworkFile->path().extension().compare(".txt") != 0)) { continue; }

    // TODO: determine if the file is currently in the league array:

    if (verbose >= 6)
    {
      std::cerr << "INF " << __FILE__ << "." << __LINE__ <<
        ": Loading network at \"" << cNetworkFile->path() << "\"...\n";
    }

    numTotal++;

    // read file from disk
    ranked_neuralNet cNetwork(neuralNet(), 0, netSettings, expSettings, tSettings);
    isSuccessful = PkIO::inputRankedNetwork(cNetworkFile->path(), cNetwork);

    if (!isSuccessful)
    {
      std::cerr << "ERR " << __FILE__ << "." << __LINE__ <<
        ": Unable to read network from disk at \"" << cNetworkFile->path() << "\"!\n";
      continue;
    }

    cNetwork.setStateSaved();

    // make sure the hashes are correct, and if they're not redo them:
    {
      uint64_t oldHash = cNetwork.getHash();
      cNetwork.generateHash();
      if (cNetwork.getHash() != oldHash)
      {
        numIncorrectHashes++;
        // recreate the network's naming if the hash was incorrect
        cNetwork.defineName();

        if (verbose >= 5)
        {
          std::cerr << "WAR " << __FILE__ << "." << __LINE__ <<
            " Network at \"" << cNetworkFile->path() << "\" hash does not match its specified network!\n";
        }
      } // endOf hash incorrect
    }

    // determine if an evaluator exists for this type of network:
    if (!cNetwork.hasEvaluator())
    {
      std::cerr << "ERR " << __FILE__ << "." << __LINE__ <<
        ": No loaded evaluators exist for given network topology!\n";
      return false;
    }

    // don't add a duplicate network:
    if (isInNetworks(cNetwork)){ isSuccessful = false; }

    if (isSuccessful)
    {
      numLoaded++;
      networks.push_back(cNetwork);
    }
  } // endOf foreach file

  if (verbose >= 0)
  {
    std::clog << "loaded " << numLoaded <<
      " of " << numTotal <<
      " networks ( " << numIncorrectHashes <<
      " hashes rebuilt ) from \"" << networkPath << "\"!\n";
    std::clog.flush();
  }

  return true;
} // endOf loadElements


bool TrainerNeural::saveNetworkPopulation()
{
  // assure path exists:
  boost::filesystem::path pPath(networkPath);

  // determine if folder exists:
  if (!boost::filesystem::exists(pPath))
  {
    if (verbose >= 6)
    {
      std::cerr << "INF " << __FILE__ << "." << __LINE__ <<
        ": A network folder was not found at location \"" << pPath << "\", creating one...\n";
    }

    if (!boost::filesystem::create_directory(pPath))
    {
      std::cerr << "ERR " << __FILE__ << "." << __LINE__ <<
        ": A network folder could not be created at \"" << pPath << "\"!\n";
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
    std::cout << "Backing up networks to disk at location \"" << networkPath << "\"...\n";
  }

  size_t numDestroyed = 0;
  size_t numFailed = 0;
  size_t numTotal = 0;
  size_t numTotalSaved = 0;
  size_t numSaved = 0;
  size_t numUpdated = 0;

  numTotal = networks.size();

  // destroy outdated elements in directory:
  for ( boost::filesystem::directory_iterator cNetworkFile(pPath), endNetworkFile; cNetworkFile != endNetworkFile; ++cNetworkFile)
  {
    // ignore directories
    if (boost::filesystem::is_directory(*cNetworkFile)) { continue; }

    // make sure extension is txt, ignore all others:
    if ((cNetworkFile->path().extension().compare(".txt") != 0)) { continue; }

    const std::string cNetworkFileStem = cNetworkFile->path().stem().string();

    // make sure extension is of the form NETx<HASH>.txt
    if (cNetworkFileStem.size() != 20) { continue; }

    // hash from name of network:
    uint64_t hash;
    {
      std::stringstream hexNameBuf(cNetworkFileStem);
      // determine hash:
      hexNameBuf.seekg(4);
      if (!(hexNameBuf >> std::hex >> hash ))
      {
        std::cerr << "ERR " << __FILE__ << "." << __LINE__ <<
        ": Unable to determine hash from filename of \"" << cNetworkFile->path() << "\"!\n";
        continue;
      }
    }

    numTotalSaved++;

    // if we haven't modified the network vector yet:
    size_t iNetwork = findInNetworks(hash);
    if ((iNetwork != SIZE_MAX) || (generationsCompleted == 0))
    {
      // DO NOT delete network:
      continue;
    }

    if (verbose >= 6)
    {
      std::cerr << "INF " << __FILE__ << "." << __LINE__ <<
        "Deleting network at \"" << cNetworkFile->path() << "\"...\n";
    }

    if (!boost::filesystem::remove(cNetworkFile->path()))
    {
      std::cerr << "ERR " << __FILE__ << "." << __LINE__ <<
        ": Could not delete file at \"" << cNetworkFile->path() << "\"!\n";
      return false;
    }

    numDestroyed++;
  } // endOf destroy outdated elements

  // add elements to directory:
  saveElements(pPath, networks, numSaved, numUpdated, numFailed);

  if (numFailed > 0)
  {
    std::cerr << "ERR " << __FILE__ << "." << __LINE__ <<
      "Backing up " << numFailed <<
      " of " << numTotal <<
      " FAILED!\n";
    // todo: stop backup on failure mode:
  }

  if (verbose >= 0)
  {
    std::clog
      << "Saved " << numSaved
      << " ( updated " << numUpdated
      << " ) of " << numTotal
      << " networks, deleted " << numDestroyed
      << " of " << numTotalSaved
      << " from \"" << networkPath << "\" !\n";
    std::clog.flush();
  }

  return true;
} // endOf savePopulation
