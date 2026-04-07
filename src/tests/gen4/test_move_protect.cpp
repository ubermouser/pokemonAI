#include "engine_test.hpp"

class ProtectTest : public Gen4EngineTest {
 protected:
  void SetUp() override {
    Gen4EngineTest::SetUp();
    // Team A: Blissey with Protect, Softboiled, Psychic, Substitute
    auto team_a = TeamNonVolatile()
        .addPokemon(PokemonNonVolatile()
          .setBase(pokedex_->pokemon("blissey"))
          .addMove(pokedex_->move("protect"))
          .addMove(pokedex_->move("softboiled"))
          .addMove(pokedex_->move("psychic"))
          .addMove(pokedex_->move("substitute"))
          .setLevel(50));

    // Team B: Gengar with various moves
    auto team_b = TeamNonVolatile()
        .addPokemon(PokemonNonVolatile()
          .setBase(pokedex_->pokemon("gengar"))
          .addMove(pokedex_->move("sludge bomb")) // Special Damaging move
          .addMove(pokedex_->move("shadow ball")) // Special Damaging move
          .addMove(pokedex_->move("hypnosis")) // Status move
          .addMove(pokedex_->move("night shade")) // Fixed damage
          .setLevel(50));

    environment_nv = EnvironmentNonvolatile(team_a, team_b, true);
    engine_->setEnvironment(environment_nv);
  }
};

TEST_F(ProtectTest, BlocksDamage) {
  // Blissey uses Protect (Priority 4). Gengar uses Sludge Bomb (Move 0).
  // Sludge Bomb should deal 0 damage.

  // Get initial HP
  uint32_t initialHP = engine_->initialState().teammate(0, 0).getHP();

  auto turn1 = engine_->updateState(engine_->initialState(), Action::moveAlly(0, 0), Action::move(0));
  auto blissey = turn1.where1().teammate(0, 0);

  EXPECT_EQ(blissey.getHP(), initialHP);
}

TEST_F(ProtectTest, BlocksStatus) {
  // Blissey uses Protect. Gengar uses Hypnosis (Move 2).
  auto turn1 = engine_->updateState(engine_->initialState(), Action::moveAlly(0, 0), Action::move(2));
  auto blissey = turn1.where1().teammate(0, 0);

  EXPECT_EQ(blissey.getStatusAilment(), AIL_NV_NONE);
}

TEST_F(ProtectTest, ConsecutiveUseChance) {
  // Turn 1: Protect (100%).
  auto turn1 = engine_->updateState(engine_->initialState(), Action::moveAlly(0, 0), Action::wait());
  EXPECT_EQ(turn1.size(), 1UL); // Should be 1 state (100% success)

  auto state1 = turn1.where1();
  // Verify counter is 1.
  EXPECT_EQ(state1.teammate(0,0).status().cTeammate.protect_counter, 1U);

  // Turn 2: Protect vs Sludge Bomb.
  // Should split into Success (Block) and Failure (Damage).
  auto turn2_attack = engine_->updateState(state1, Action::moveAlly(0, 0), Action::move(0));

  // Now states should differ:
  // State A: Blocked damage (Success). High HP.
  // State B: Took damage (Failure). Low HP.

  EXPECT_GT(turn2_attack.size(), 1UL);

  bool foundBlocked = false;
  bool foundDamage = false;

  uint32_t maxHP = state1.teammate(0,0).getHP();

  for (size_t i=0; i<turn2_attack.size(); ++i) {
      auto s = turn2_attack.at(i);
      if (s.teammate(0,0).getHP() == maxHP) foundBlocked = true;
      else foundDamage = true;
  }

  EXPECT_TRUE(foundBlocked);
  EXPECT_TRUE(foundDamage);
}

TEST_F(ProtectTest, CounterResetsOnOtherMove) {
  // Turn 1: Protect. Counter -> 1.
  auto turn1 = engine_->updateState(engine_->initialState(), Action::moveAlly(0, 0), Action::wait());
  auto state1 = turn1.where1();
  EXPECT_EQ(state1.teammate(0,0).status().cTeammate.protect_counter, 1U);

  // Turn 2: Softboiled (Move 1). Counter -> 0.
  auto turn2 = engine_->updateState(state1, Action::moveAlly(1, 0), Action::wait());
  auto state2 = turn2.where1();
  EXPECT_EQ(state2.teammate(0,0).status().cTeammate.protect_counter, 0U);

  // Turn 3: Protect. Should be 100% success (Counter -> 1).
  auto turn3 = engine_->updateState(state2, Action::moveAlly(0, 0), Action::move(0)); // Against Sludge Bomb
  // Should block all damage. No split (or at least no damage taken).

  uint32_t hp = state2.teammate(0,0).getHP(); // (Healed HP)

  for (size_t i=0; i<turn3.size(); ++i) {
      // Allow for some minor diffs due to RNG but Sludge Bomb damage is significant
      EXPECT_EQ(turn3.at(i).teammate(0,0).getHP(), hp);
  }
}
