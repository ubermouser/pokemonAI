#include <gtest/gtest.h>

#include <limits>
#include <memory>
#include <sstream>
#include <unordered_set>

#include "gen4/engine_test.hpp"
#include "pokemonai/evaluator_simple.h"
#include "pokemonai/planner_human.h"
#include "pokemonai/planner_max.h"
#include "pokemonai/planner_maximin.h"
#include "pokemonai/planner_negamax.h"
#include "pokemonai/planner_random.h"
#include "pokemonai/planner_softmax.h"

class PlannerTest : public Gen4EngineTest {
 protected:
  void SetUp() override {
    Gen4EngineTest::SetUp();
    engine_->setAllowInvalidMoves(NeoPkCU::ActionValidationMethod::FULL);

    initialize_logger(spdlog::level::trace);

    EvaluatorSimple::Config eval_config;
    eval_config.canMoveBias = 0.0;
    eval_config.movesBias = 0.0;
    evaluator_ = std::make_shared<EvaluatorSimple>(eval_config);

    auto team_a = TeamNonVolatile()
        .addPokemon(PokemonNonVolatile()
          .setBase(pokedex_->pokemon("gengar"))
          .addMove(pokedex_->move("explosion"))
          .addMove(pokedex_->move("focus blast"))
          .setLevel(100)
          .setIV(FV_SPEED, 31)) // ensure Gengar wins the speed tie
        .addPokemon(PokemonNonVolatile()
          .setBase(pokedex_->pokemon("metagross"))
          .addMove(pokedex_->move("agility"))
          .addMove(pokedex_->move("meteor mash"))
          .setLevel(100));
    auto team_b = TeamNonVolatile()
        .addPokemon(PokemonNonVolatile()
          .setBase(pokedex_->pokemon("alakazam"))
          .addMove(pokedex_->move("recover"))
          .addMove(pokedex_->move("psychic"))
          .setLevel(100))
        .addPokemon(PokemonNonVolatile()
          .setBase(pokedex_->pokemon("pikachu")));
    environment_ = std::make_shared<EnvironmentNonvolatile>(team_a, team_b, true);
    engine_->setEnvironment(environment_);
  }

  std::shared_ptr<EnvironmentNonvolatile> environment_;
  std::shared_ptr<Evaluator> evaluator_;
};

TEST_F(PlannerTest, MaxPlannerChoosesGreedyOption) {
  PlannerMax::Config cfg;
  cfg.maxDepth = 1;
  cfg.verbosity = 4;
  cfg.maxTime = std::numeric_limits<double>::infinity();
  std::array<PlannerMax, 2> planners {PlannerMax(cfg), PlannerMax(cfg)};
  for (auto& planner: planners) {
    planner
        .setEvaluator(evaluator_)
        .setTeam(static_cast<TEAM>(&planner - planners.begin()))
        .setEngine(engine_)
        .setEnvironment(environment_)
        .initialize();
  }

  auto agent_result = planners[TEAM_A].generateSolution(engine_->initialState());
  auto other_result = planners[TEAM_B].generateSolution(engine_->initialState());

  EXPECT_EQ(agent_result.bestAgentAction().begin()->second, Action::moveEnemy(1, 0));
  EXPECT_LE(agent_result.best().numNodes, 7);
  EXPECT_EQ(agent_result.best().depth, 1);

  EXPECT_EQ(other_result.bestAgentAction().begin()->second, Action::moveEnemy(1, 0));
  EXPECT_LE(other_result.best().numNodes, 3);
  EXPECT_EQ(other_result.best().depth, 1);
  // agents do not take into account the enemy's move
}


TEST_F(PlannerTest, MaximinPlannerChooses1PlyOption) {
  PlannerMaximin::Config cfg;
  cfg.maxDepth = 1;
  cfg.verbosity = 4;
  cfg.maxTime = std::numeric_limits<double>::infinity();
  std::array<PlannerMaximin, 2> planners{
      PlannerMaximin(cfg), PlannerMaximin(cfg)};
  for (auto& planner: planners) {
    planner
        .setEvaluator(evaluator_)
        .setTeam(static_cast<TEAM>(&planner - planners.begin()))
        .setEngine(engine_)
        .setEnvironment(environment_)
        .initialize();
  }

  auto agent_result = planners[TEAM_A].generateSolution(engine_->initialState());
  auto other_result = planners[TEAM_B].generateSolution(engine_->initialState());

  EXPECT_EQ(agent_result.bestAgentAction().begin()->second, Action::moveAdjacent(0));
  EXPECT_LE(agent_result.best().numNodes, 21);
  EXPECT_EQ(agent_result.best().depth, 1);
  EXPECT_FLOAT_EQ(agent_result.bestFitness() + other_result.bestFitness(), 1.0);
  // other agent best move is ambiguous
}


TEST_F(PlannerTest, MaximinPlannerChooses2PlyOption) {
  PlannerMaximin::Config cfg;
  cfg.maxDepth = 2;
  cfg.verbosity = 4;
  cfg.maxTime = std::numeric_limits<double>::infinity();
  std::array<PlannerMaximin, 2> planners{
      PlannerMaximin(cfg), PlannerMaximin(cfg)};
  for (auto& planner: planners) {
    planner
        .setEvaluator(evaluator_)
        .setTeam(static_cast<TEAM>(&planner - planners.begin()))
        .setEngine(engine_)
        .setEnvironment(environment_)
        .initialize();
  }

  auto agent_result = planners[TEAM_A].generateSolution(engine_->initialState());
  auto other_result = planners[TEAM_B].generateSolution(engine_->initialState());

  EXPECT_EQ(agent_result.bestAgentAction().begin()->second, Action::swap(1));
  EXPECT_LE(agent_result.best().numNodes, 243);
  EXPECT_EQ(agent_result.best().depth, 2);
  EXPECT_FLOAT_EQ(agent_result.bestFitness() + other_result.bestFitness(), 1.0);
  EXPECT_LE(other_result.best().numNodes, 243);
  EXPECT_EQ(other_result.best().depth, 2);
  // other agent best move is ambiguous
}


class PlannerNPlyTest : public PlannerTest {
protected:
  void SetUp() override {
    // Though team_a can defeat team_b in 2 turns, team_a might choose not to end
    // the game because team_a cannot be hurt by team_b. This test ensures that
    // planners are greedy in the face of equally good options.
    PlannerTest::SetUp();
    auto team_a = TeamNonVolatile()
        .addPokemon(PokemonNonVolatile()
          .setBase(pokedex_->pokemon("gengar"))
          .addMove(pokedex_->move("double team"))
          .addMove(pokedex_->move("drain punch")));
    auto team_b = TeamNonVolatile()
        .addPokemon(PokemonNonVolatile()
          .setBase(pokedex_->pokemon("rattata"))
          .addMove(pokedex_->move("return")));
    environment_ = std::make_shared<EnvironmentNonvolatile>(team_a, team_b, true);
    engine_->setEnvironment(environment_);
  }
};


class PlannerMaximinTest : public PlannerNPlyTest {
protected:
  void SetUp() override {
    PlannerNPlyTest::SetUp();
    PlannerMaximin::Config cfg;
    cfg.minDepth = 4;
    cfg.maxDepth = 4;  // success is possible in 2 turns
    cfg.verbosity = 4;
    cfg.maxTime = std::numeric_limits<double>::infinity();

    planner_ = std::make_shared<PlannerMaximin>(cfg);
    planner_->setEvaluator(evaluator_)
        .setTeam(TEAM_A)
        .setEngine(engine_)
        .setEnvironment(environment_)
        .initialize();
  }

  std::shared_ptr<Planner> planner_;
};


TEST_F(PlannerMaximinTest, planner_chooses_n_ply_option) {
  auto agent_result = planner_->generateSolution(engine_->initialState());

  EXPECT_EQ(agent_result.bestAgentAction().begin()->second, Action::moveEnemy(1, 0));
  // TODO: not enough opportunities to cutoff moves in the contrived example
  EXPECT_EQ(agent_result.best().numNodes, 114);
  EXPECT_FLOAT_EQ(agent_result.bestFitness(), 1.0);
}


class PlannerMinimaxTest : public PlannerNPlyTest {
 protected:
  void SetUp() override {
    PlannerNPlyTest::SetUp();
    PlannerMinimax::Config cfg;
    cfg.minDepth = 4;
    cfg.maxDepth = 4; // success is possible in 2 turns
    cfg.verbosity = 4;
    cfg.maxTime = std::numeric_limits<double>::infinity();

    planner_ = std::make_shared<PlannerMinimax>(cfg);
    planner_->setEvaluator(evaluator_)
        .setTeam(TEAM_A)
        .setEngine(engine_)
        .setEnvironment(environment_)
        .initialize();
  }

  std::shared_ptr<Planner> planner_;
};


TEST_F(PlannerMinimaxTest, planner_chooses_n_ply_option) {
  auto agent_result = planner_->generateSolution(engine_->initialState());

  EXPECT_EQ(agent_result.bestAgentAction().begin()->second, Action::moveEnemy(1, 0));
  EXPECT_EQ(agent_result.best().numNodes, 114);
  EXPECT_FLOAT_EQ(agent_result.bestFitness(), 1.0);
}


class PlannerNegamaxTest : public PlannerNPlyTest {
 protected:
  void SetUp() override {
    PlannerNPlyTest::SetUp();
    PlannerNegamax::Config cfg;
    cfg.minDepth = 4;
    cfg.maxDepth = 4;  // success is possible in 2 turns
    cfg.verbosity = 4;
    cfg.transposition_table_size = 256;
    cfg.maxTime = std::numeric_limits<double>::infinity();

    planner_ = std::make_shared<PlannerNegamax>(cfg);
    planner_->setEvaluator(evaluator_)
        .setTeam(TEAM_A)
        .setEngine(engine_)
        .setEnvironment(environment_)
        .initialize();
  }

  std::shared_ptr<Planner> planner_;
};


TEST_F(PlannerNegamaxTest, planner_chooses_n_ply_option) {
  auto agent_result = planner_->generateSolution(engine_->initialState());

  EXPECT_EQ(agent_result.bestAgentAction().begin()->second, Action::moveEnemy(1, 0));
  EXPECT_EQ(agent_result.best().numNodes, 57);
  EXPECT_FLOAT_EQ(agent_result.bestFitness(), 1.0);
}


class PlannerSoftmaxTest : public PlannerTest {
 protected:
  void SetUp() override {
    PlannerTest::SetUp();

    PlannerSoftmax::Config softmax_cfg;
    softmax_cfg.temperature = 0.0;
    softmax_cfg.maxDepth = 1;
    softmax_planner_ = std::make_unique<PlannerSoftmax>(softmax_cfg);
    softmax_planner_->setEvaluator(evaluator_)
        .setTeam(TEAM_A)
        .setEngine(engine_)
        .setEnvironment(environment_)
        .initialize();

    PlannerSoftmax::Config softmax_cfg_high_temp;
    softmax_cfg_high_temp.temperature = 10.0;
    softmax_cfg_high_temp.maxDepth = 1;
    softmax_planner_high_temp_ =
        std::make_unique<PlannerSoftmax>(softmax_cfg_high_temp);
    softmax_planner_high_temp_->setEvaluator(evaluator_)
        .setTeam(TEAM_A)
        .setEngine(engine_)
        .setEnvironment(environment_)
        .initialize();

    PlannerMaximin::Config maximin_cfg;
    maximin_cfg.maxDepth = 1;
    maximin_planner_ = std::make_unique<PlannerMaximin>(maximin_cfg);
    maximin_planner_->setEvaluator(evaluator_)
        .setTeam(TEAM_A)
        .setEngine(engine_)
        .setEnvironment(environment_)
        .initialize();
  }

  std::unique_ptr<PlannerSoftmax> softmax_planner_;
  std::unique_ptr<PlannerSoftmax> softmax_planner_high_temp_;
  std::unique_ptr<PlannerMaximin> maximin_planner_;
};


TEST_F(PlannerSoftmaxTest, TemperatureZeroFallbacksToMaximin) {
  auto softmax_result =
      softmax_planner_->generateSolution(engine_->initialState());
  auto maximin_result =
      maximin_planner_->generateSolution(engine_->initialState());

  EXPECT_EQ(softmax_result.bestAgentAction().begin()->second, maximin_result.bestAgentAction().begin()->second);
}


TEST_F(PlannerSoftmaxTest, HighTemperatureDiversifiesActions) {
  srand(42);

  std::unordered_set<Action> chosen_actions;
  for (int i = 0; i < 50; ++i) {
    auto result =
        softmax_planner_high_temp_->generateSolution(engine_->initialState());
    chosen_actions.insert(result.bestAgentAction().begin()->second);
  }

  // We should see both moves being picked at least once
  EXPECT_GT(chosen_actions.size(), 1);
}
