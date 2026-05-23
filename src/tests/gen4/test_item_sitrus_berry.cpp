#include "engine_test.hpp"

class SitrusBerryTest : public Gen4EngineTest {
 protected:
  void SetUp() override {
    Gen4EngineTest::SetUp();

    // Attacker: Level 40 Mew with Seismic Toss (deals exactly 40 HP damage)
    // Defender: Level 100 Bulbasaur with Sitrus Berry (Max HP = 200)
    auto team_a = TeamNonVolatile().addPokemon(
        PokemonNonVolatile()
            .setBase(pokedex_->pokemon("mew"))
            .addMove(pokedex_->move("seismic toss"))
            .setLevel(40));

    auto team_b = TeamNonVolatile().addPokemon(
        PokemonNonVolatile()
            .setBase(pokedex_->pokemon("bulbasaur"))
            .setInitialItem(pokedex_->item("sitrus berry"))
            .addMove(pokedex_->move("growl"))
            .setLevel(100));

    environment_nv = EnvironmentNonvolatile(team_a, team_b, true);
    engine_->setEnvironment(environment_nv);
  }

  // Turn 1 setup: Mew uses Seismic Toss (40 damage)
  PossibleEnvironments setupTurn1() {
    return engine_->updateState(
        engine_->initialState(), Action::move(0), Action::wait());
  }

  // Turn 2 setup: Mew uses Seismic Toss (40 damage)
  PossibleEnvironments setupTurn2() {
    return engine_->updateState(
        setupTurn1().where1(), Action::move(0), Action::wait());
  }

  // Turn 3 setup: Mew uses Seismic Toss (40 damage)
  PossibleEnvironments setupTurn3() {
    return engine_->updateState(
        setupTurn2().where1(), Action::move(0), Action::wait());
  }
};


TEST_F(SitrusBerryTest, Turn1DoesNotTriggerAboveFiftyPercent) {
  auto turn1_result = setupTurn1();
  auto state_1 = turn1_result.where1();
  uint32_t max_hp = state_1.teammate(TEAM_B, 0).nv().getMaxHP();

  // After Turn 1: HP: 200 -> 160 (80% HP). Sitrus Berry should NOT trigger.
  EXPECT_EQ(state_1.teammate(TEAM_B, 0).getHP(), max_hp - 40);
  EXPECT_TRUE(state_1.teammate(TEAM_B, 0).hasItem());
}


TEST_F(SitrusBerryTest, Turn2DoesNotTriggerAboveFiftyPercent) {
  auto turn2_result = setupTurn2();
  auto state_2 = turn2_result.where1();
  uint32_t max_hp = state_2.teammate(TEAM_B, 0).nv().getMaxHP();

  // After Turn 2: HP: 160 -> 120 (60% HP). Sitrus Berry should NOT trigger.
  EXPECT_EQ(state_2.teammate(TEAM_B, 0).getHP(), max_hp - 80);
  EXPECT_TRUE(state_2.teammate(TEAM_B, 0).hasItem());
}


TEST_F(SitrusBerryTest, Turn3TriggersAtOrBelowFiftyPercent) {
  auto turn3_result = setupTurn3();
  auto state_3 = turn3_result.where1();
  uint32_t max_hp = state_3.teammate(TEAM_B, 0).nv().getMaxHP();

  // After Turn 3: HP: 120 -> 80 (40% HP). Sitrus Berry triggers and heals 25%
  // (50 HP). HP ends at: 80 + 50 = 130.
  uint32_t expected_hp = max_hp - 120 + (max_hp / 4);
  EXPECT_EQ(state_3.teammate(TEAM_B, 0).getHP(), expected_hp);
}


TEST_F(SitrusBerryTest, Turn3ConsumesSitrusBerry) {
  auto turn3_result = setupTurn3();
  auto state_3 = turn3_result.where1();

  // Item should be consumed
  EXPECT_FALSE(state_3.teammate(TEAM_B, 0).hasItem());
}


TEST_F(SitrusBerryTest, Turn3LogsSitrusBerryUsage) {
  auto turn2_result = setupTurn2();
  auto turn3_result = setupTurn3();

  // Verify state transition output logs the item usage
  auto output = StateTransitionPrinter::printString(
      turn2_result.where1(), turn3_result.where1(), false);
  EXPECT_TRUE(
      output.find("bulbasaur used its sitrus berry") != std::string::npos);
}
