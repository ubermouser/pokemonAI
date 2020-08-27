#include <gtest/gtest.h>

#include <memory>

#include <inc/engine.h>
#include <inc/evaluator_simple.h>
#include <inc/pokedex_static.h>
#include <inc/pkCU.h>
#include <inc/planner_random.h>
#include <inc/planner_max.h>
#include <inc/planner_maximin.h>


class PlannerTest : public ::testing::Test {
protected:
  void SetUp() override {
    verbose = 4;
    pokedex_ = std::make_shared<PokedexStatic>();
    engine_ = std::make_shared<PkCU>();

    auto team_a = TeamNonVolatile()
        .addPokemon(PokemonNonVolatile()
          .setBase(pokedex_->getPokemon().at("charmander"))
          .addMove(pokedex_->getMoves().at("cut"))
          .addMove(pokedex_->getMoves().at("swords dance"))
          .setLevel(100));
    auto team_b = TeamNonVolatile()
        .addPokemon(PokemonNonVolatile()
          .setBase(pokedex_->getPokemon().at("bulbasaur"))
          .addMove(pokedex_->getMoves().at("cut"))
          .addMove(pokedex_->getMoves().at("charm"))
          .setLevel(100));
    environment_ = std::make_shared<EnvironmentNonvolatile>(team_a, team_b, true);
    engine_->setEnvironment(environment_);
  }

  std::shared_ptr<EnvironmentNonvolatile> environment_;
  std::shared_ptr<Pokedex> pokedex_;
  std::shared_ptr<PkCU> engine_;
};


TEST_F(PlannerTest, MaxPlannerChoosesGreedyOption) {
  PlannerMax::Config cfg;
  cfg.verbosity = 4;
  std::unique_ptr<Planner> planner = std::make_unique<PlannerMax>(cfg);
  planner->setTeam(TEAM_A)
      .setEvaluator(EvaluatorSimple())
      .setEngine(engine_)
      .setEnvironment(environment_)
      .initialize();

  auto result = planner->generateSolution(engine_->initialState());

  EXPECT_EQ(result.bestAgentAction(), AT_MOVE_0);
}


TEST_F(PlannerTest, MaximinPlannerChooses1PlyOption) {
  PlannerMaxiMin::Config cfg;
  cfg.maxDepth = 1;
  cfg.verbosity = 4;
  std::unique_ptr<Planner> planner = std::make_unique<PlannerMaxiMin>(cfg);
  planner->setTeam(TEAM_A)
      .setEvaluator(EvaluatorSimple())
      .setEngine(engine_)
      .setEnvironment(environment_)
      .initialize();

  auto result = planner->generateSolution(engine_->initialState());

  EXPECT_EQ(result.bestAgentAction(), AT_MOVE_0);
}


TEST_F(PlannerTest, MaximinPlannerChooses2PlyOption) {
  PlannerMaxiMin::Config cfg;
  cfg.maxDepth = 2;
  cfg.verbosity = 4;
  std::unique_ptr<Planner> planner = std::make_unique<PlannerMaxiMin>(cfg);
  planner->setTeam(TEAM_A)
      .setEvaluator(EvaluatorSimple())
      .setEngine(engine_)
      .setEnvironment(environment_)
      .initialize();

  auto result = planner->generateSolution(engine_->initialState());

  EXPECT_EQ(result.bestAgentAction(), AT_MOVE_0);
}