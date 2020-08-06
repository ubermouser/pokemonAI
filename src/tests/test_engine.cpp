#include <gtest/gtest.h>

#include <memory>

#include <inc/engine.h>
#include <inc/pokedex_dynamic.h>
#include <inc/pkCU.h>
#include <inc/pkIO.h>


class EngineTest : public ::testing::Test {
protected:
  void SetUp() override {
    pokedex_ = std::make_shared<PokedexDynamic>();
    auto team = PkIO::inputPlayerTeam("teams/soloTeamA.txt");
    environment_ = EnvironmentNonvolatile(team, team, true);
    engine_ = std::make_shared<PkCU>(EnvironmentNonvolatile(team, team, true));
  }

  std::shared_ptr<PokedexDynamic> pokedex_;
  EnvironmentNonvolatile environment_;
  std::shared_ptr<PkCU> engine_;
};


TEST_F(EngineTest, AcceptsMoves) {
  PossibleEnvironments result = engine_->updateState(
      engine_->initialState(), AT_MOVE_0, AT_MOVE_NOTHING);
  result.printStates(environment_);
  EXPECT_EQ(result.size(), 2);
}


/*TEST_F(EngineTest, Flinch) {
  team.teammate(0).setMove(0, pokedex->getMoves().at("brave bird"));
  engine->setEnvironment(EnvironmentNonvolatile(team, team, true));

  
}*/