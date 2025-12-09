#include <gtest/gtest.h>

#include <memory>
#include <sstream>
#include <limits>

#include "pokemonai/engine.h"
#include "pokemonai/evaluator_simple.h"
#include "pokemonai/pokedex_static.h"
#include "pokemonai/pkCU.h"
#include "pokemonai/planner_random.h"
#include "pokemonai/planner_max.h"
#include "pokemonai/planner_maximin.h"
#include "pokemonai/planner_human.h"


class PlannerTest : public ::testing::Test {
protected:
  void SetUp() override {
    verbose = 4;
    pokedex_ = std::make_shared<PokedexStatic>();
    engine_ = std::make_shared<PkCU>();
    EvaluatorSimple::Config eval_config;
    eval_config.canMoveBias = 0.0;
    eval_config.movesBias = 0.0;
    evaluator_ = std::make_shared<EvaluatorSimple>(eval_config);

    auto team_a = TeamNonVolatile()
        .addPokemon(PokemonNonVolatile()
          .setBase(pokedex_->getPokemon().at("gengar"))
          .addMove(pokedex_->getMoves().at("explosion"))
          .addMove(pokedex_->getMoves().at("focus blast"))
          .setLevel(100)
          .setIV(FV_SPEED, 31)) // ensure Gengar wins the speed tie
        .addPokemon(PokemonNonVolatile()
          .setBase(pokedex_->getPokemon().at("metagross"))
          .addMove(pokedex_->getMoves().at("agility"))
          .addMove(pokedex_->getMoves().at("meteor mash"))
          .setLevel(100));
    auto team_b = TeamNonVolatile()
        .addPokemon(PokemonNonVolatile()
          .setBase(pokedex_->getPokemon().at("alakazam"))
          .addMove(pokedex_->getMoves().at("recover"))
          .addMove(pokedex_->getMoves().at("psychic"))
          .setLevel(100))
        .addPokemon(PokemonNonVolatile()
          .setBase(pokedex_->getPokemon().at("pikachu")));
    environment_ = std::make_shared<EnvironmentNonvolatile>(team_a, team_b, true);
    engine_->setEnvironment(environment_);
  }

  std::shared_ptr<EnvironmentNonvolatile> environment_;
  std::shared_ptr<Pokedex> pokedex_;
  std::shared_ptr<PkCU> engine_;
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
        .setTeam(&planner - planners.begin())
        .setEngine(engine_)
        .setEnvironment(environment_)
        .initialize();
  }

  auto agent_result = planners[TEAM_A].generateSolution(engine_->initialState());
  auto other_result = planners[TEAM_B].generateSolution(engine_->initialState());

  EXPECT_EQ(agent_result.bestAgentAction(), Action::move(1));
  EXPECT_LE(agent_result.best().numNodes, 7);
  EXPECT_EQ(agent_result.best().depth, 1);

  EXPECT_EQ(other_result.bestAgentAction(), Action::move(1));
  EXPECT_LE(other_result.best().numNodes, 3);
  EXPECT_EQ(other_result.best().depth, 1);
  // agents do not take into account the enemy's move
}


TEST_F(PlannerTest, MaximinPlannerChooses1PlyOption) {
  PlannerMaxiMin::Config cfg;
  cfg.maxDepth = 1;
  cfg.verbosity = 4;
  cfg.maxTime = std::numeric_limits<double>::infinity();
  std::array<PlannerMaxiMin, 2> planners {PlannerMaxiMin(cfg), PlannerMaxiMin(cfg)};
  for (auto& planner: planners) {
    planner
        .setEvaluator(evaluator_)
        .setTeam(&planner - planners.begin())
        .setEngine(engine_)
        .setEnvironment(environment_)
        .initialize();
  }

  auto agent_result = planners[TEAM_A].generateSolution(engine_->initialState());
  auto other_result = planners[TEAM_B].generateSolution(engine_->initialState());

  EXPECT_EQ(agent_result.bestAgentAction(), Action::move(0));
  EXPECT_LE(agent_result.best().numNodes, 21);
  EXPECT_EQ(agent_result.best().depth, 1);
  EXPECT_FLOAT_EQ(agent_result.bestFitness() + other_result.bestFitness(), 1.0);
  // other agent best move is ambiguous
}


TEST_F(PlannerTest, MaximinPlannerChooses2PlyOption) {
  PlannerMaxiMin::Config cfg;
  cfg.maxDepth = 2;
  cfg.verbosity = 4;
  cfg.maxTime = std::numeric_limits<double>::infinity();
  std::array<PlannerMaxiMin, 2> planners {PlannerMaxiMin(cfg), PlannerMaxiMin(cfg)};
  for (auto& planner: planners) {
    planner
        .setEvaluator(evaluator_)
        .setTeam(&planner - planners.begin())
        .setEngine(engine_)
        .setEnvironment(environment_)
        .initialize();
  }

  auto agent_result = planners[TEAM_A].generateSolution(engine_->initialState());
  auto other_result = planners[TEAM_B].generateSolution(engine_->initialState());

  EXPECT_EQ(agent_result.bestAgentAction(), Action::swap(1));
  EXPECT_LE(agent_result.best().numNodes, 235);
  EXPECT_EQ(agent_result.best().depth, 2);
  EXPECT_FLOAT_EQ(agent_result.bestFitness() + other_result.bestFitness(), 1.0);
  EXPECT_LE(other_result.best().numNodes, 235);
  EXPECT_EQ(other_result.best().depth, 2);
  // other agent best move is ambiguous
}


TEST_F(PlannerTest, MaximinPlannerChoosesNPlyOption) {
  PlannerMaxiMin::Config cfg;
  cfg.minDepth = 4;
  cfg.maxDepth = 4; // success is possible in 2 turns
  cfg.verbosity = 4;
  cfg.maxTime = std::numeric_limits<double>::infinity();
  auto team_a = TeamNonVolatile()
      .addPokemon(PokemonNonVolatile()
        .setBase(pokedex_->getPokemon().at("gengar"))
        .addMove(pokedex_->getMoves().at("double team"))
        .addMove(pokedex_->getMoves().at("drain punch")));
  auto team_b = TeamNonVolatile()
      .addPokemon(PokemonNonVolatile()
        .setBase(pokedex_->getPokemon().at("rattata"))
        .addMove(pokedex_->getMoves().at("return")));
  auto environment = EnvironmentNonvolatile(team_a, team_b, true);

  auto planner = PlannerMaxiMin(cfg);
  planner
      .setEvaluator(evaluator_)
      .setTeam(TEAM_A)
      .setEngine(engine_)
      .setEnvironment(environment)
      .initialize();

  auto agent_result = planner.generateSolution(engine_->initialState());

  EXPECT_EQ(agent_result.bestAgentAction(), Action::move(1));
  EXPECT_LE(agent_result.best().numNodes, 114);
  EXPECT_FLOAT_EQ(agent_result.bestFitness(), 1.0);
}


TEST_F(PlannerTest, HumanPlannerActionReader) {
  Action result;
  {
    std::istringstream input("m2");
    PlannerHuman planner(PlannerHuman::Config(), input);
    planner.setTeam(TEAM_A).setEngine(engine_).setEnvironment(environment_).initialize();
    result = planner.generateSolution(engine_->initialState()).bestAgentAction();
    EXPECT_EQ(result, Action::move(1));
  }
  {
    std::istringstream input("S5");
    input >> result;
    EXPECT_EQ(result, Action::swap(4));
  }
  {
    std::istringstream input("m2-4");
    input >> result;
    EXPECT_EQ(result, Action::moveAlly(1, 3));
  }
  {
    std::stringstream input; input << Action::moveAlly(1, 3);
    input >> result;
    EXPECT_EQ(result, Action::moveAlly(1, 3));
  }
  {
    std::istringstream input("garbage");
    input >> result;
    EXPECT_FALSE(input);
  }
}


// TODO(@drendleman) - test that minimax planner does the same things that maximin planner does