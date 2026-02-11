#include "engine_test.hpp"

class TriAttackTest : public Gen4EngineTest {
 protected:
  void SetUp() override {
    Gen4EngineTest::SetUp();

    // Team A: Magneton with Tri Attack
    auto team_a = TeamNonVolatile()
        .addPokemon(PokemonNonVolatile()
          .setBase(pokedex_->pokemon("magneton"))
          .addMove(pokedex_->move("tri attack"))
          .setLevel(100));

    // Team B: Chansey (target) - high HP
    auto team_b = TeamNonVolatile()
        .addPokemon(PokemonNonVolatile()
          .setBase(pokedex_->pokemon("chansey"))
          .setLevel(100));

    environment_nv = EnvironmentNonvolatile(team_a, team_b, true);
    engine_->setEnvironment(environment_nv);
  }
};

TEST_F(TriAttackTest, InflictsStatus) {
  // Use Tri Attack (Move 0) against opponent
  auto results = engine_->updateState(engine_->initialState(), Action::move(0), Action::wait());

  // Filter states where secondary effect occurred for Team A (index 0)
  auto status_states = results.whereStatus(0);

  // Check that we have states with secondary effects
  // Tri Attack has 20% secondary chance, so we should expect some states.
  EXPECT_FALSE(status_states.empty()) << "Tri Attack secondary effect did not trigger in any environment.";

  bool burned = false;
  bool frozen = false;
  bool paralyzed = false;

  for (const auto& state : status_states) {
    // Check the status of the target (Team B / index 1)
    // Note: getTeam(1) gets the team structure, then getPKV() gets the active pokemon.
    uint32_t status = state.getTeam(1).getPKV().getStatusAilment();

    if (status == AIL_NV_BURN) burned = true;
    if (status == AIL_NV_FREEZE) frozen = true;
    if (status == AIL_NV_PARALYSIS) paralyzed = true;
  }

  EXPECT_TRUE(burned) << "Should have inflicted Burn in at least one scenario";
  EXPECT_TRUE(frozen) << "Should have inflicted Freeze in at least one scenario";
  EXPECT_TRUE(paralyzed) << "Should have inflicted Paralysis in at least one scenario";
}
