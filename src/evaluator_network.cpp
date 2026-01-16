#include "pokemonai/evaluator_network.h"

#include <assert.h>

#include <algorithm>
#include <boost/math/special_functions/fpclassify.hpp>
#include <fstream>
#include <sstream>

#include "pokemonai/environment_nonvolatile.h"
#include "pokemonai/environment_volatile.h"
#include "pokemonai/fp_compare.h"
#include "pokemonai/move.h"
#include "pokemonai/pokemon_base.h"
#include "pokemonai/pokemon_nonvolatile.h"
#include "pokemonai/pokemon_volatile.h"
#include "pokemonai/team_nonvolatile.h"
#include "pokemonai/team_volatile.h"
#include "pokemonai/trainable_neural_net.h"
#include "pokemonai/type.h"

namespace po = boost::program_options;

// featureVector implementations
void EvaluatorNetwork::seed(
    neuralNet& cNet, const ConstEnvironmentVolatile& env, size_t iTeam) const {
  assert(cNet.numInputs() == inputSize() && cNet.numOutputs() == outputSize());
  seed(cNet.inputBegin(), env, iTeam);
}


// featureVector_impl implementations
void featureVector_impl::generateBestMoves(const EnvironmentNonvolatile& envNV, bestMoveOrders_t& iBestMoves, bestMoveDamages_t& dBestMoves) {
    for (size_t iTeam = 0; iTeam < 2; ++iTeam) {
        const TeamNonVolatile& cTNV = envNV.getTeam(iTeam);
        const TeamNonVolatile& tTNV = envNV.getOtherTeam(iTeam);

        for (size_t iTeammate = 0; iTeammate != 6; ++iTeammate) {
            for (size_t iOTeammate = 0; iOTeammate != 6; ++iOTeammate) {
                iBestMoves[iTeam][iTeammate][iOTeammate].fill(UINT8_MAX);
                dBestMoves[iTeam][iTeammate][iOTeammate].fill(0.0f);
            }

            if (iTeammate >= cTNV.getNumTeammates()) continue;

            const PokemonNonVolatile& cPKNV = cTNV.teammate(iTeammate);
            const PokemonBase& cPKB = cPKNV.getBase();
            float levelModifier = (((((float)cPKNV.getLevel() * 2.0f) / 5.0f) + 2.0f) / 50.0f) * 0.85f;

            for (size_t iOTeammate = 0; iOTeammate != 6; ++iOTeammate) {
                if (iOTeammate >= tTNV.getNumTeammates()) continue;

                const PokemonNonVolatile& tPKNV = tTNV.teammate(iOTeammate);
                const PokemonBase& tPKB = tPKNV.getBase();

                float physicalDamage = levelModifier * ((float)cPKNV.getFV_base(FV_ATTACK)) / ((float)tPKNV.getFV_base(FV_DEFENSE));
                float specialDamage = levelModifier * ((float)cPKNV.getFV_base(FV_SPATTACK)) / ((float)tPKNV.getFV_base(FV_SPDEFENSE));

                std::array<bool, 4> valid; valid.fill(true);
                for (size_t iNMove = 0; iNMove != cPKNV.getNumMoves(); ++iNMove) {
                    float bestDamage = -std::numeric_limits<float>::infinity();
                    uint8_t iBestDamage = UINT8_MAX;

                    for (size_t iMove = 0; iMove != cPKNV.getNumMoves(); ++iMove) {
                        if (!valid[iMove]) continue;
                        const Move& cMove = cPKNV.getMove_base(iMove);
                        const Type& cType = cMove.getType();
                        bool hasStab = ((&cPKB.getType(0) == &cType) || (&cPKB.getType(1) == &cType));
                        float typeStabBonus = (float)cType.getModifier(tPKB.getType(0)) * (float)cType.getModifier(tPKB.getType(1)) * (hasStab?1.5f:1.0f);
                        float damageTypeBonus = (cMove.getDamageType()==ATK_PHYSICAL)?physicalDamage:(cMove.getDamageType()==ATK_SPECIAL)?specialDamage:0.0f;
                        float simpleDamage = (float)cMove.getPower() * damageTypeBonus * typeStabBonus;
                        if (mostlyGT(simpleDamage, 0.0f)) simpleDamage += 0.125f;
                        if (simpleDamage > bestDamage) { bestDamage = simpleDamage; iBestDamage = (uint8_t)iMove; }
                    }
                    bestDamage = scale(bestDamage, (float)tPKNV.getMaxHP(), 0.0f);
                    valid[iBestDamage] = false;
                    iBestMoves[iTeam][iTeammate][iOTeammate][iNMove] = iBestDamage;
                    dBestMoves[iTeam][iTeammate][iOTeammate][iNMove] = bestDamage;
                }
            }
        }
    }
}

void featureVector_impl::generateOrders(const bestMoveDamages_t& dBestMoves, orders_t& orders) {
    std::array< std::array< uint8_t , 6> , 2> preOrders;
    for (size_t iTeam = 0; iTeam < 2; ++iTeam) {
        std::array<bool, 6> valid; valid.fill(true);
        for (size_t iNTeammate = 0; iNTeammate != 6; ++iNTeammate) {
            float bestCoverage = -std::numeric_limits<float>::infinity();
            size_t iBestCoverage = SIZE_MAX;
            for (size_t iTeammate = 0; iTeammate != 6; ++iTeammate) {
                if (!valid[iTeammate]) continue;
                float currentCoverage = 0.0;
                for (size_t iOTeammate = 0; iOTeammate != 6; ++iOTeammate) {
                    currentCoverage += dBestMoves[iTeam][iTeammate][iOTeammate][0];
                }
                if (currentCoverage > bestCoverage) { bestCoverage = currentCoverage; iBestCoverage = iTeammate; }
            }
            valid[iBestCoverage] = false;
            preOrders[iTeam][iNTeammate] = (uint8_t)iBestCoverage;
        }
    }
    for (size_t iTeam = 0; iTeam < 2; ++iTeam) {
        for (size_t iTeammate = 0; iTeammate != 6; ++iTeammate) {
            auto it = orders[iTeam][iTeammate].begin();
            *it++ = (uint8_t)iTeammate;
            for (size_t iNTeammate = 0; iNTeammate != 6; ++iNTeammate) {
                if (iTeammate == preOrders[iTeam][iNTeammate]) continue;
                *it++ = preOrders[iTeam][iNTeammate];
            }
        }
    }
}

boost::program_options::options_description EvaluatorNetwork::Config::options(
    const std::string& category, std::string prefix) {
  auto desc = Evaluator::Config::options(category, prefix);

  if (prefix.size() > 0) { prefix.append("-"); }
  // clang-format off
  desc.add_options()
    ((prefix + "model").c_str(), 
    po::value<std::string>(&modelPath), 
    "Path to a pre-trained model file");
  // clang-format on
  desc.add(
      netConfig.options(category + " [TrainableNeuralNet]", prefix + "net"));
  return desc;
}

EvaluatorNetwork::EvaluatorNetwork(
    const Config& cfg, size_t inputSize, size_t outputSize)
    : Evaluator(cfg),
      cfg_(cfg),
      network_(std::make_shared<TrainableNeuralNet>(
          cfg.netConfig, inputSize, outputSize)) {}

EvaluatorNetwork::EvaluatorNetwork(const neuralNet& network, const Config& cfg)
    : Evaluator(cfg),
      cfg_(cfg),
      network_(std::make_shared<neuralNet>(network)) {}

EvaluatorNetwork::EvaluatorNetwork(const EvaluatorNetwork& other)
    : Evaluator(other),
      cfg_(other.cfg_),
      network_(other.network_),
      iBestMoves_(other.iBestMoves_),
      dBestMoves_(other.dBestMoves_),
      orders_(other.orders_) {}

EvaluatorNetwork::~EvaluatorNetwork() {
}

EvaluatorNetwork& EvaluatorNetwork::initialize() {
  Evaluator::initialize();

  if (!network_ && !cfg_.modelPath.empty()) {
    auto net = std::make_shared<neuralNet>();
    std::ifstream iFile(cfg_.modelPath, std::ios::binary);
    if (!iFile) {
      throw std::invalid_argument(fmt::format(
          "EvaluatorNetwork: could not open model file {}", cfg_.modelPath));
    }
    if (!net->input(iFile)) {
      throw std::invalid_argument(fmt::format(
          "EvaluatorNetwork: failed to load model from {}", cfg_.modelPath));
    }
    network_ = net;
    updateIdent();
  }

  if (!network_) {
    throw std::invalid_argument("EvaluatorNetwork: network undefined");
  }
  if (!network_->isInitialized()) {
    throw std::invalid_argument("EvaluatorNetwork: network not initialized");
  }
  if (network_->numInputs() != inputSize() ||
      network_->numOutputs() != outputSize()) {
    throw std::invalid_argument(fmt::format(
        "EvaluatorNetwork requires input-{} (has {}), output-{} (has {})!",
        inputSize(),
        network_->numInputs(),
        outputSize(),
        network_->numOutputs()));
  }
  return *this;
}

void EvaluatorNetwork::setNetwork(const std::shared_ptr<neuralNet>& network) {
  network_ = network;
  updateIdent();
}

EvaluatorNetwork& EvaluatorNetwork::setEnvironment(
    const std::shared_ptr<const EnvironmentNonvolatile>& env) {
  Evaluator::setEnvironment(env);
  // nv_ is already set by Evaluator::setEnvironment
  generateBestMoves();
  generateOrders();
  if (network_) network_->clearInput();
  return *this;
}

EvalResult EvaluatorNetwork::calculateFitness(const ConstEnvironmentVolatile& env, size_t iTeam) const {
    return calculateFitness(*network_, env, iTeam);
}

EvalResult EvaluatorNetwork::calculateFitness(neuralNet& cNet, const ConstEnvironmentVolatile& env, size_t iTeam) const {
  seed(cNet.inputBegin(), env, iTeam);
  cNet.feedForward();
  fpType fitness = *cNet.outputBegin();
  fitness = std::max(
      (fpType)0.0,
      std::min((fpType)1.0, scale(fitness, (fpType)0.85, (fpType)0.15)));
  return EvalResult{Fitness{fitness}};
}

void EvaluatorNetwork::generateBestMoves() {
    assert(nv_ != nullptr);
    featureVector_impl::generateBestMoves(*nv_, iBestMoves_, dBestMoves_);
}

void EvaluatorNetwork::generateOrders() {
    featureVector_impl::generateOrders(dBestMoves_, orders_);
}

void EvaluatorNetwork::updateIdent() {
    std::ostringstream nameStr;
    nameStr << "neural_Evaluator(" << inputSize() << "." << outputSize() << ")-"
         << (network_ ? network_->getName() : "NULLNETWORK");
    setName(nameStr.str());
}
