#include "engine_test.hpp"

class DestinyBondTest : public Gen4EngineTest {
 protected:
  void SetUp() override {
    Gen4EngineTest::SetUp();

    // Team A: Faster pokemon (Gengar)
    auto team_a_nv = TeamNonVolatile()
        .addPokemon(PokemonNonVolatile()
          .setBase(pokedex_->pokemon("gengar"))
          .addMove(pokedex_->move("destiny bond"))
          .addMove(pokedex_->move("shadow ball"))
          .setLevel(100));

    // Team B: Slower pokemon (Snorlax)
    auto team_b_nv = TeamNonVolatile()
        .addPokemon(PokemonNonVolatile()
          .setBase(pokedex_->pokemon("snorlax"))
          .addMove(pokedex_->move("crunch")) // Dark move to hit Gengar
          .addMove(pokedex_->move("tackle")) // Normal move
          .setLevel(100));

    environment_nv = EnvironmentNonvolatile(team_a_nv, team_b_nv, true);
    engine_->setEnvironment(environment_nv);
  }
};

TEST_F(DestinyBondTest, TriggerOnKill) {
  // Turn 1: Gengar uses Destiny Bond, Snorlax uses Crunch
  auto possible_envs = engine_->updateState(
    engine_->initialState(), Action::move(0), Action::move(0));
  
  // Gengar is fragile, Crunch from Snorlax should OHKO or at least kill.
  auto fainted_envs = possible_envs.where([](const ConstEnvironmentPossible& env) {
    return env.teammate(0, 0).getHP() == 0;
  });
  
  ASSERT_FALSE(fainted_envs.empty()) << "Gengar did not faint, test cannot verify Destiny Bond trigger";

  for (const auto& env : fainted_envs) {
    // Snorlax should also be fainted due to Destiny Bond
    EXPECT_EQ(env.teammate(1, 0).getHP(), 0) << "Snorlax should have fainted due to Destiny Bond";
  }
}

TEST_F(DestinyBondTest, NoTriggerNoKill) {
  // Snorlax uses Block (status move, no damage)
  auto possible_envs = engine_->updateState(
    engine_->initialState(), Action::move(0), Action::move(1));
  
  auto env = possible_envs.where1();
  EXPECT_GT(env.teammate(0, 0).getHP(), 0);
  EXPECT_GT(env.teammate(1, 0).getHP(), 0);
  EXPECT_TRUE(env.teammate(0, 0).status().cTeammate.destinyBond);
}

TEST_F(DestinyBondTest, ConsecutiveUseFails) {
  // Turn 1: Gengar uses Destiny Bond, Snorlax uses Block
  auto envs1 = engine_->updateState(
    engine_->initialState(), Action::move(0), Action::move(1));
  auto state1 = envs1.where1();
  EXPECT_TRUE(state1.teammate(0, 0).status().cTeammate.destinyBond);

  // Turn 2: Gengar uses Destiny Bond again
  auto envs2 = engine_->updateState(
    state1.getEnv(), Action::move(0), Action::move(1));
  auto state2 = envs2.where1();
  
  // In Gen 4, Destiny Bond fails if used consecutively.
  EXPECT_FALSE(state2.teammate(0, 0).status().cTeammate.destinyBond);
}

TEST_F(DestinyBondTest, EffectWearsOff) {
  // Turn 1: Gengar uses Destiny Bond, Snorlax uses Block
  auto envs1 = engine_->updateState(
    engine_->initialState(), Action::move(0), Action::move(1));
  auto state1 = envs1.where1(); 
  EXPECT_TRUE(state1.teammate(0, 0).status().cTeammate.destinyBond);

  // Turn 2: Gengar uses Shadow Ball, Snorlax uses Crunch and kills Gengar
  auto envs2 = engine_->updateState(
    state1.getEnv(), Action::move(1), Action::move(0));
  
  auto fainted_envs = envs2.where([](const ConstEnvironmentPossible& env) {
    return env.teammate(0, 0).getHP() == 0;
  });
  
  ASSERT_FALSE(fainted_envs.empty());
  
  for (const auto& env : fainted_envs) {
    // Snorlax should NOT be fainted because Destiny Bond wore off
    EXPECT_GT(env.teammate(1, 0).getHP(), 0) << "Snorlax should not have fainted, Destiny Bond should have worn off";
  }
}
