#include <gtest/gtest.h>

#include <memory>

#include <inc/engine.h>
#include <inc/pokedex_dynamic.h>
#include <inc/pkCU.h>
#include <inc/pkIO.h>

TEST(EngineTest, AcceptsMoves) {
  PokedexDynamic pokedex;
  TeamNonVolatile team = PkIO::inputPlayerTeam("teams/soloTeamA.txt");
  EnvironmentNonvolatile nonvolatileState{team, team, true};

  PkCU engine(nonvolatileState);

  PossibleEnvironments result = engine.updateState(
      engine.initialState(), AT_MOVE_0, AT_MOVE_NOTHING);
  result.printStates(nonvolatileState);
  EXPECT_GE(result.size(), 2);
}