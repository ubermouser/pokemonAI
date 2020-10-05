#include <gtest/gtest.h>

#include <memory>
#include <sstream>
#include <limits>

#include <inc/engine.h>
#include <inc/evaluator_simple.h>
#include <inc/pokedex_static.h>
#include <inc/pkCU.h>
#include <inc/planner_random.h>
#include <inc/planner_max.h>
#include <inc/planner_maximin.h>
#include <inc/planner_human.h>


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
          .addMove(pokedex_->getMoves().at("focus blast")))
        .addPokemon(PokemonNonVolatile()
          .setBase(pokedex_->getPokemon().at("metagross"))
          .addMove(pokedex_->getMoves().at("agility"))
          .addMove(pokedex_->getMoves().at("meteor mash")));
    auto team_b = TeamNonVolatile()
        .addPokemon(PokemonNonVolatile()
          .setBase(pokedex_->getPokemon().at("alakazam"))
          .addMove(pokedex_->getMoves().at("recover"))
          .addMove(pokedex_->getMoves().at("psychic")))
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
  EXPECT_EQ(other_result.bestAgentAction(), Action::move(1));
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
  EXPECT_EQ(agent_result.bestOtherAction(), other_result.bestAgentAction());
  EXPECT_EQ(other_result.bestOtherAction(), agent_result.bestAgentAction());
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
  EXPECT_EQ(agent_result.bestOtherAction(), other_result.bestAgentAction());
  EXPECT_EQ(other_result.bestOtherAction(), agent_result.bestAgentAction());
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


// TODO(@drendleman) - test that planner ;chooses a guaranteed winning move in the shortest depth
// TODO(@drendleman) - test that minimax planner does the same things that maximin planner does