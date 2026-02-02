#include "engine_test.hpp"

class BatonPassTest : public Gen4EngineTest {
 protected:
  void SetUp() override {
    Gen4EngineTest::SetUp();

    // Team A: Scizor (Baton Pass, Swords Dance, Substitute) and Torterra
    auto teamA = TeamNonVolatile()
        .addPokemon(PokemonNonVolatile()
          .setBase(pokedex_->pokemon("scizor"))
          .addMove(pokedex_->move("baton pass"))
          .addMove(pokedex_->move("swords dance"))
          .addMove(pokedex_->move("substitute"))
          .setInitialItem(pokedex_->item("leftovers"))
          .setLevel(100))
        .addPokemon(PokemonNonVolatile()
          .setBase(pokedex_->pokemon("torterra"))
          .addMove(pokedex_->move("earthquake"))
          .setLevel(100));

    // Team B: Gengar (Confuse Ray)
    auto teamB = TeamNonVolatile()
        .addPokemon(PokemonNonVolatile()
          .setBase(pokedex_->pokemon("gengar"))
          .addMove(pokedex_->move("confuse ray"))
          .addMove(pokedex_->move("shadow ball"))
          .setLevel(100));

    environment_nv = EnvironmentNonvolatile(teamA, teamB, true);
    engine_->setEnvironment(environment_nv);
  }
};

TEST_F(BatonPassTest, PassesBoosts) {
  // Turn 1: Scizor uses Swords Dance (+2 Atk)
  auto turn1 = engine_->updateState(
      engine_->initialState(), Action::move(1), Action::wait());

  auto scizor_boosted = turn1.where1Hit(0);
  EXPECT_EQ(scizor_boosted.teammate(0, 0).getBoost(FV_ATTACK), 2);

  // Turn 2: Scizor uses Baton Pass to Torterra (Friendly 1)
  // Baton Pass is index 0. Torterra is Friendly 1.
  auto turn2 = engine_->updateState(
      turn1.where1(), Action::moveAlly(0, 1), Action::wait());

  auto torterra_switched_in = turn2.where1Hit(0);

  // Verify switch happened
  EXPECT_EQ(torterra_switched_in.getTeam(0).getICPKV(), 1);
  EXPECT_TRUE(torterra_switched_in.teammate(0, 1).data().active);
  EXPECT_FALSE(torterra_switched_in.teammate(0, 0).data().active);

  // Verify Boosts passed (+2 Atk)
  EXPECT_EQ(torterra_switched_in.teammate(0, 1).getBoost(FV_ATTACK), 2);
}

TEST_F(BatonPassTest, PassesSubstitute) {
  // Turn 1: Scizor uses Substitute
  auto turn1 = engine_->updateState(
      engine_->initialState(), Action::move(2), Action::wait());

  auto scizor_sub = turn1.where1Hit(0);
  EXPECT_GT(scizor_sub.teammate(0, 0).status().cTeammate.substitute, 0);

  // Turn 2: Scizor uses Baton Pass to Torterra
  auto turn2 = engine_->updateState(
      turn1.where1(), Action::moveAlly(0, 1), Action::wait());

  auto torterra_switched_in = turn2.where1Hit(0);

  // Verify switch
  EXPECT_EQ(torterra_switched_in.getTeam(0).getICPKV(), 1);

  // Verify Substitute passed
  EXPECT_GT(torterra_switched_in.teammate(0, 1).status().cTeammate.substitute, 0);
}

TEST_F(BatonPassTest, DoesNotPassConfusion) {
  // Turn 1: Gengar uses Confuse Ray on Scizor
  auto turn1 = engine_->updateState(
      engine_->initialState(), Action::wait(), Action::move(0));

  auto scizor_confused = turn1.where1Hit(1); // Gengar (Team 1) hits

  // Verify Scizor (Team 0) is confused
  EXPECT_GT(scizor_confused.teammate(0, 0).status().cTeammate.confused, 0);

  // Turn 2: Scizor uses Baton Pass to Torterra
  auto turn2 = engine_->updateState(
      turn1.where1(), Action::moveAlly(0, 1), Action::wait());

  auto switched_states = turn2.whereSwitch(0);
  ASSERT_FALSE(switched_states.empty())
      << "Scizor never managed to Baton Pass (confusion blocked all?)";

  for (const auto& state : switched_states) {
    auto torterra = state.teammate(0, 1);
    // Verify Confusion NOT passed
    EXPECT_EQ(torterra.status().cTeammate.confused, 0);
  }
}

TEST_F(BatonPassTest, NormalSwitchResetsBoosts) {
  // Turn 1: Scizor uses Swords Dance (+2 Atk)
  auto turn1 = engine_->updateState(
      engine_->initialState(), Action::move(1), Action::wait());

  auto scizor_boosted = turn1.where1Hit(0);
  EXPECT_EQ(scizor_boosted.teammate(0, 0).getBoost(FV_ATTACK), 2);

  // Turn 2: Scizor switches manually to Torterra (Swap 1)
  auto turn2 = engine_->updateState(
      turn1.where1(), Action::swap(1), Action::wait());

  auto torterra_switched_in = turn2.where1();

  // Verify switch
  EXPECT_EQ(torterra_switched_in.getTeam(0).getICPKV(), 1);

  // Verify Boosts RESET (0 Atk)
  EXPECT_EQ(torterra_switched_in.teammate(0, 1).getBoost(FV_ATTACK), 0);
}
