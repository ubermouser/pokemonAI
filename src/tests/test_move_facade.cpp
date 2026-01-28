#include "engine_test.hpp"


class FacadeTest : public Gen4EngineTest {
 protected:
  void SetUp() override {
    Gen4EngineTest::SetUp();

    // Team A: Mew with Facade
    auto team_a = TeamNonVolatile()
        .addPokemon(PokemonNonVolatile()
          .setBase(pokedex_->pokemon("mew"))
          .addMove(pokedex_->move("facade"))
          .setLevel(100));

    // Team B: Mewtwo with status moves
    auto team_b = TeamNonVolatile()
        .addPokemon(PokemonNonVolatile()
          .setBase(pokedex_->pokemon("mewtwo"))
          .addMove(pokedex_->move("will-o-wisp")) // 0: Burn
          .addMove(pokedex_->move("thunder wave")) // 1: Paralysis
          .addMove(pokedex_->move("toxic"))        // 2: Bad Poison
          .addMove(pokedex_->move("recover"))      // 3: Dummy
          .setLevel(100));

    environment_nv = EnvironmentNonvolatile(team_a, team_b, true);
    engine_->setEnvironment(environment_nv);

    // Calculate base damage for reuse (no status)
    auto normal_damage_env = engine_->updateState(engine_->initialState(), Action::move(0), Action::wait());
    normal_damage =
        normal_damage_env.where1().getTeam(1).getPKV().getMissingHP();
  }

  // Helper to run a turn where opponent uses a status move, then user uses Facade
  uint32_t getDamageAfterStatusMove(const Action& move) {
    // Team 0 (User) waits, Team 1 (Opponent) uses status move
    // Use the engine's internal NV state to ensure pointer matching (required by guardNonvolatileState)
    auto result_t1 = engine_->updateState(engine_->initialState(), Action::wait(), move);
    // Turn 2: User uses Facade (Move 0), Opponent waits
    auto result_t2 = engine_->updateState(
        result_t1.where1(), Action::move(0), Action::wait());
    return result_t2.where1().getTeam(1).getPKV().getMissingHP();
  }

  uint32_t normal_damage;
};


TEST_F(FacadeTest, BoostedByPoison) {
  // Use Toxic as a proxy for poison since Facade treats them similarly for boost
  uint32_t boosted_damage = getDamageAfterStatusMove(Action::move(2));
  
  // Facade doubles power, so damage should be roughly doubled
  EXPECT_NEAR(boosted_damage, normal_damage * 2., 1.);
}


TEST_F(FacadeTest, BoostedByParalysis) {
  uint32_t boosted_damage = getDamageAfterStatusMove(Action::move(1));
  
  EXPECT_NEAR(boosted_damage, normal_damage * 2., 1.);
}


TEST_F(FacadeTest, BoostedByBurn) {
  uint32_t boosted_damage = getDamageAfterStatusMove(Action::move(0));
  
  // Burn normally halves attack, but Facade ignores this drop AND doubles power.
  EXPECT_NEAR(boosted_damage, normal_damage * 2., 1.);
}
