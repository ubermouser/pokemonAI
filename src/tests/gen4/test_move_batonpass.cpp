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

  PossibleEnvironments useSwordsDance() {
    return engine_->updateState(
        engine_->initialState(), Action::moveAlly(1, 0), Action::wait());
  }

  PossibleEnvironments useBatonPassAfterSwordsDance(const PossibleEnvironments& turn1) {
    return engine_->updateState(
        turn1.where1(), Action::moveAlly(0, 1), Action::wait());
  }

  PossibleEnvironments useSubstitute() {
    return engine_->updateState(
        engine_->initialState(), Action::moveAlly(2, 0), Action::wait());
  }

  PossibleEnvironments useBatonPassAfterSubstitute(const PossibleEnvironments& turn1) {
    return engine_->updateState(
        turn1.where1(), Action::moveAlly(0, 1), Action::wait());
  }

  PossibleEnvironments receiveConfuseRay() {
    return engine_->updateState(
        engine_->initialState(), Action::wait(), Action::move(0));
  }

  PossibleEnvironments useBatonPassAfterConfuseRay(const PossibleEnvironments& turn1) {
    return engine_->updateState(
        turn1.where1(), Action::moveAlly(0, 1), Action::wait());
  }

  PossibleEnvironments manualSwitchAfterSwordsDance(const PossibleEnvironments& turn1) {
    return engine_->updateState(
        turn1.where1(), Action::swap(1), Action::wait());
  }
};

TEST_F(BatonPassTest, PassesBoosts) {
  auto turn1 = useSwordsDance();
  auto scizor_boosted = turn1.where1Hit(0);
  EXPECT_EQ(scizor_boosted.teammate(0, 0).getBoost(FV_ATTACK), 2);

  auto turn2 = useBatonPassAfterSwordsDance(turn1);
  auto torterra_switched_in = turn2.where1Hit(0);

  // Verify switch happened
  EXPECT_TRUE(torterra_switched_in.teammate(0, 1).isActive());
  EXPECT_FALSE(torterra_switched_in.teammate(0, 0).isActive());

  // Verify Boosts passed (+2 Atk)
  EXPECT_EQ(torterra_switched_in.teammate(0, 1).getBoost(FV_ATTACK), 2);
}

TEST_F(BatonPassTest, PassesSubstitute) {
  auto turn1 = useSubstitute();
  auto scizor_sub = turn1.where1Hit(0);
  EXPECT_GT(scizor_sub.teammate(0, 0).status().substitute, 0);

  auto turn2 = useBatonPassAfterSubstitute(turn1);
  auto torterra_switched_in = turn2.where1Hit(0);

  // Verify switch
  EXPECT_TRUE(torterra_switched_in.teammate(0, 1).isActive());
  EXPECT_FALSE(torterra_switched_in.teammate(0, 0).isActive());

  // Verify Substitute passed
  EXPECT_GT(torterra_switched_in.teammate(0, 1).status().substitute, 0);
}

TEST_F(BatonPassTest, DoesNotPassConfusion) {
  auto turn1 = receiveConfuseRay();
  auto scizor_confused = turn1.where1Hit(1);  // Gengar (Team 1) hits

  // Verify Scizor (Team 0) is confused
  EXPECT_GT(scizor_confused.teammate(0, 0).status().confused, 0);

  auto turn2 = useBatonPassAfterConfuseRay(turn1);
  auto switched_states = turn2.whereSwitch(0);
  ASSERT_FALSE(switched_states.empty())
      << "Scizor never managed to Baton Pass (confusion blocked all?)";

  for (const auto& state : switched_states) {
    auto torterra = state.teammate(0, 1);
    // Verify Confusion NOT passed
    EXPECT_EQ(torterra.status().confused, 0);
  }
}

TEST_F(BatonPassTest, NormalSwitchResetsBoosts) {
  auto turn1 = useSwordsDance();
  auto scizor_boosted = turn1.where1Hit(0);
  EXPECT_EQ(scizor_boosted.teammate(0, 0).getBoost(FV_ATTACK), 2);

  auto turn2 = manualSwitchAfterSwordsDance(turn1);
  auto torterra_switched_in = turn2.where1();

  // Verify switch
  EXPECT_TRUE(torterra_switched_in.teammate(0, 1).isActive());
  EXPECT_FALSE(torterra_switched_in.teammate(0, 0).isActive());

  // Verify Boosts RESET (0 Atk)
  EXPECT_EQ(torterra_switched_in.teammate(0, 1).getBoost(FV_ATTACK), 0);
}
