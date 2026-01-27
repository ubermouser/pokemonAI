#include <gtest/gtest.h>

#include <sstream>

#include "engine_test.hpp"
#include "pokemonai/item.h"
#include "pokemonai/move_volatile.h"
#include "pokemonai/planner_human.h"
#include "pokemonai/pokemon_volatile.h"
#include "pokemonai/state_transition_printer.h"


class PlannerHumanTest : public Gen4EngineTest {
 protected:
  void SetUp() override {
    Gen4EngineTest::SetUp();
    engine_->setAllowInvalidMoves(false);

    PokemonNonVolatile gengarNV;
    gengarNV.setBase(pokedex_->pokemon("gengar"))
            .setAbility(pokedex_->ability("levitate"))
            .addMove(pokedex_->move("explosion"))
            .addMove(pokedex_->move("focus blast"))
            .setLevel(100)
            .setIV(FV_SPEED, 31);
    gengarNV.setName("gengar");

    PokemonNonVolatile metagrossNV;
    metagrossNV.setBase(pokedex_->pokemon("metagross"))
               .addMove(pokedex_->move("agility"))
               .addMove(pokedex_->move("meteor mash"))
               .setLevel(100);
    metagrossNV.setName("metagross");

    PokemonNonVolatile alakazamNV;
    alakazamNV.setBase(pokedex_->pokemon("alakazam"))
              .addMove(pokedex_->move("recover"))
              .addMove(pokedex_->move("psychic"))
              .setLevel(100);
    alakazamNV.setName("alakazam");

    PokemonNonVolatile pikachuNV;
    pikachuNV.setBase(pokedex_->pokemon("pikachu"));
    pikachuNV.setName("pikachu");

    auto team_a = TeamNonVolatile()
        .addPokemon(gengarNV)
        .addPokemon(metagrossNV);
    auto team_b = TeamNonVolatile()
        .addPokemon(alakazamNV)
        .addPokemon(pikachuNV);

    environment_ = std::make_shared<EnvironmentNonvolatile>(team_a, team_b, true);
    engine_->setEnvironment(environment_);
  }

  std::shared_ptr<EnvironmentNonvolatile> environment_;
};


TEST_F(PlannerHumanTest, MoveVolatileOperatorOutput) {
  const auto& moves = pokedex_->getMoves();
  ASSERT_TRUE(moves.count("tackle"));
  const Move& tackle = moves.at("tackle");
  
  MoveNonVolatile mNV(tackle);
  MoveVolatileData data;
  data.PPcurrent = 35;
  data.status_nonvolatile = 0;
  
  ConstMoveVolatile cMV(mNV, data);
  
  std::stringstream ss;
  ss << cMV;
  
  std::string output = ss.str();
  SCOPED_TRACE(output);

  // Example output: "tackle" [normal] Physical Pwr: 35 PP: 35/56
  EXPECT_TRUE(output.find("\"tackle\"") != std::string::npos);
  EXPECT_TRUE(output.find("normal") != std::string::npos);
  EXPECT_TRUE(output.find("Physical") != std::string::npos);
  EXPECT_TRUE(output.find("Pwr: 35") != std::string::npos);
  EXPECT_TRUE(output.find("PP: 35") != std::string::npos);
}


TEST_F(PlannerHumanTest, HumanPlannerActionReader) {
  Action result;
  {
    std::istringstream input("m2");
    PlannerHuman::Config cfg;
    cfg.maxDepth = 0;
    PlannerHuman planner(cfg, input);
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


TEST_F(PlannerHumanTest, PokemonVolatilePrettyPrint) {
  const TeamNonVolatile& teamA = environment_->getTeam(TEAM_A);
  const PokemonNonVolatile& gengarNV = teamA.teammate(0);
  
  PokemonVolatileData data;
  data.HPcurrent = gengarNV.getMaxHP();
  data.active = 1;
  data.status_nonvolatile = AIL_NV_NONE;
  data.iHeldItem = Item::no_item->index_;

  TeamStatus status{};
  
  PokemonVolatile gengarV(gengarNV, data, status);
  
  testing::internal::CaptureStdout();
  gengarV.prettyPrint("PRE:", ":SUF");
  std::string output = testing::internal::GetCapturedStdout();
  
  SCOPED_TRACE(output);
  EXPECT_TRUE(output.find("gengar") != std::string::npos);
  EXPECT_TRUE(output.find("HP:") != std::string::npos);
  EXPECT_TRUE(output.find("230/230") != std::string::npos);
  EXPECT_TRUE(output.find("ghost") != std::string::npos);
  EXPECT_TRUE(output.find("poison") != std::string::npos);
  EXPECT_TRUE(output.find("Normal") != std::string::npos);
  EXPECT_TRUE(output.find("I: None") != std::string::npos);
  EXPECT_TRUE(output.find("levitate") != std::string::npos);
  EXPECT_TRUE(output.find("explosion") != std::string::npos);
  EXPECT_TRUE(output.find("focus blast") != std::string::npos);
}


// TODO: move these to individual move/ability/item tests
TEST_F(PlannerHumanTest, PokemonVolatilePrettyPrintWithStatusAndItem) {
  const TeamNonVolatile& teamA = environment_->getTeam(TEAM_A);
  const PokemonNonVolatile& gengarNV = teamA.teammate(0);
  
  PokemonVolatileData data;
  data.HPcurrent = 130; // approx half
  data.active = 1;
  data.status_nonvolatile = AIL_NV_BURN;
  
  // Need a real item index for test
  const auto& items = pokedex_->getItems();
  ASSERT_FALSE(items.empty());
  const Item& lifeOrb = items.at("life orb");
  data.iHeldItem = lifeOrb.index_;

  TeamStatus status{};
  
  PokemonVolatile gengarV(gengarNV, data, status);
  
  testing::internal::CaptureStdout();
  gengarV.prettyPrint();
  std::string output = testing::internal::GetCapturedStdout();
  
  SCOPED_TRACE(output);
  EXPECT_TRUE(output.find("gengar") != std::string::npos);
  EXPECT_TRUE(output.find("130/230") != std::string::npos);
  EXPECT_TRUE(output.find("BRN") != std::string::npos);
  EXPECT_TRUE(output.find("life orb") != std::string::npos);
}


TEST_F(PlannerHumanTest, PokemonVolatileOperatorOutput) {
  const TeamNonVolatile& teamA = environment_->getTeam(TEAM_A);
  const PokemonNonVolatile& gengarNV = teamA.teammate(0);
  
  PokemonVolatileData data;
  data.HPcurrent = gengarNV.getMaxHP();
  data.active = 1;
  data.status_nonvolatile = AIL_NV_PARALYSIS;

  TeamStatus status{};
  status.cTeammate.boosts.B_SPA = 2; // +2 SpA
  
  ConstPokemonVolatile gengarV(gengarNV, data, status);
  
  std::stringstream ss;
  ss << gengarV;
  std::string output = ss.str();
  
  SCOPED_TRACE(output);
  // Example output: "gengar"-"gengar" 230/230 PAR +2spa
  EXPECT_TRUE(output.find("\"gengar\"-\"gengar\"") != std::string::npos);
  EXPECT_TRUE(output.find("230/230") != std::string::npos);
  EXPECT_TRUE(output.find("PAR") != std::string::npos);
  EXPECT_TRUE(output.find("+2spa") != std::string::npos);
}


TEST_F(PlannerHumanTest, StateTransitionPrinterDamage) {
  EnvironmentVolatileData oldState =
      EnvironmentVolatileData::create(*environment_);
  EnvironmentPossibleData newState = EnvironmentPossibleData::create(oldState);

  // Apply damage to gengar on team A
  newState.env.teams[TEAM_A].teammates[0].HPcurrent = 200;  // was 230
  newState.flags.bits.team1.hit = 1;                        // Team B hit
  newState.flags.bits.team1.movesFirst = 1;

  testing::internal::CaptureStdout();
  StateTransitionPrinter::print(
      std::cout,
      ConstEnvironmentPossible{
          *environment_, EnvironmentPossibleData::create(oldState, false)},
      ConstEnvironmentPossible{*environment_, newState});
  std::string output = testing::internal::GetCapturedStdout();

  SCOPED_TRACE(output);
  EXPECT_TRUE(output.find("gengar") != std::string::npos);
  EXPECT_TRUE(output.find("lost 30 HP") != std::string::npos);
}


TEST_F(PlannerHumanTest, StateTransitionPrinterCritAndStatus) {
  EnvironmentVolatileData oldState =
      EnvironmentVolatileData::create(*environment_);
  EnvironmentPossibleData newState = EnvironmentPossibleData::create(oldState);

  // Team A moves first, crits
  newState.flags.bits.team0.movesFirst = 1;
  newState.flags.bits.team0.hit = 1;
  newState.flags.bits.team0.crit = 1;

  // Team B active pokemon (alakazam) gets burned
  newState.env.teams[TEAM_B].teammates[0].status_nonvolatile = AIL_NV_BURN;

  testing::internal::CaptureStdout();
  StateTransitionPrinter::print(
      std::cout,
      ConstEnvironmentPossible{
          *environment_, EnvironmentPossibleData::create(oldState, false)},
      ConstEnvironmentPossible{*environment_, newState});
  std::string output = testing::internal::GetCapturedStdout();

  SCOPED_TRACE(output);
  EXPECT_TRUE(output.find("critical hit") != std::string::npos);
  EXPECT_TRUE(output.find("alakazam") != std::string::npos);
  EXPECT_TRUE(output.find("burned") != std::string::npos);
}


TEST_F(PlannerHumanTest, StateTransitionPrinterFaintAndSwitch) {
  EnvironmentVolatileData oldState =
      EnvironmentVolatileData::create(*environment_);
  EnvironmentPossibleData newState = EnvironmentPossibleData::create(oldState);

  // Gengar faints
  newState.env.teams[TEAM_A].teammates[0].HPcurrent = 0;

  // Team A switches to metagross
  newState.flags.bits.team0.switched = 1;
  newState.env.teams[TEAM_A].status.nonvolatile.iCPokemon = 1;

  testing::internal::CaptureStdout();
  StateTransitionPrinter::print(
      std::cout,
      ConstEnvironmentPossible{
          *environment_, EnvironmentPossibleData::create(oldState, false)},
      ConstEnvironmentPossible{*environment_, newState});
  std::string output = testing::internal::GetCapturedStdout();

  SCOPED_TRACE(output);
  EXPECT_TRUE(output.find("gengar") != std::string::npos);
  EXPECT_TRUE(output.find("fainted") != std::string::npos);
  EXPECT_TRUE(output.find("metagross") != std::string::npos);
}


TEST_F(PlannerHumanTest, StateTransitionPrinterDamageOnSwitch) {
  // Team A switches from gengar to metagross while Team B attacks with psychic
  auto switch_and_attack = engine_->updateState(
      engine_->initialState(), Action::swap(1), Action::move(1));

  std::string output = StateTransitionPrinter::printString(
      engine_->initialState(),
      ConstEnvironmentPossible{*environment_, switch_and_attack.at(0)},
      /*withStyle=*/false);

  SCOPED_TRACE(output);
  EXPECT_TRUE(output.find("metagross lost") != std::string::npos)
      << "Expected to find 'metagross lost ... HP' in output";
}
