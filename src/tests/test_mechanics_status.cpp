#include "engine_test.hpp"

class StatusTest : public EngineTest {
protected:
  void SetUp() override {
    EngineTest::SetUp();

    // Setup: Machamp (High Attack, Physical) vs Weezing (High Def, learns Will-O-Wisp)
    // Weezing uses Will-O-Wisp on Machamp.

    // Team A: Machamp
    auto team_a = TeamNonVolatile()
        .addPokemon(PokemonNonVolatile()
          .setBase(pokedex_->pokemon("machamp"))
          .addMove(pokedex_->move("cross chop")) // Physical move
          .setLevel(100));

    // Team B: Weezing
    auto team_b = TeamNonVolatile()
        .addPokemon(PokemonNonVolatile()
          .setBase(pokedex_->pokemon("weezing"))
          .addMove(pokedex_->move("will-o-wisp")) // Burn move
          .addMove(pokedex_->move("sludge bomb")) // Attack move
          .setLevel(100));

    environment_nv = EnvironmentNonvolatile(team_a, team_b, true);
    engine_->setEnvironment(environment_nv);

    // Initial state: Unburned
    // To setup Burned state: Team B uses Will-O-Wisp on Team A. Team A waits.
    setup_burned = engine_->updateState(engine_->initialState(), Action::wait(), Action::move(0));
  }

  PossibleEnvironments setup_burned;
};

TEST_F(StatusTest, BurnEffect) {
  // Scenario 1: Unburned Machamp attacks (from initial state)
  // Cross Chop: 100 Power, Physical
  // Team B waits.
  auto result_normal = engine_->updateState(
      engine_->initialState(), Action::move(0), Action::wait());

  // Scenario 2: Burned Machamp attacks
  // Team A uses Cross Chop. Team B waits.

  // Helper to find a burned state
  ConstEnvironmentVolatile burnedState = setup_burned.at(0).getEnv();
  bool foundBurned = false;
  for (size_t i = 0; i < setup_burned.size(); ++i) {
      // Machamp is Team 0, Teammate 0.
      if (setup_burned.at(i).getEnv().getTeam(0).teammate(0).getStatusAilment() == AIL_NV_BURN) {
          burnedState = setup_burned.at(i).getEnv();
          foundBurned = true;
          break;
      }
  }

  ASSERT_TRUE(foundBurned) << "Failed to setup burned state via Will-O-Wisp";

  auto result_burned = engine_->updateState(
      burnedState, Action::move(0), Action::wait());

  // Helper to get average damage dealt to opponent
  auto getAvgDamageDealt = [](const PossibleEnvironments& results, int defenderTeam) {
    double totalDamage = 0;
    double totalProb = 0;
    for (size_t i = 0; i < results.size(); ++i) {
        auto res = results.at(i);
        double prob = res.getProbability().to_double();
        int missingHP = res.getEnv().getTeam(defenderTeam).teammate(0).getMissingHP();
        totalDamage += missingHP * prob;
        totalProb += prob;
    }
    return totalProb > 0 ? totalDamage / totalProb : 0.0;
  };

  // We check damage on Team B (Weezing)
  double damageNormal = getAvgDamageDealt(result_normal, TEAM_B);
  double damageBurned = getAvgDamageDealt(result_burned, TEAM_B);

  std::cout << "Normal Damage: " << damageNormal << std::endl;
  std::cout << "Burned Damage: " << damageBurned << std::endl;

  // Burn should reduce physical damage by 50%
  EXPECT_LT(damageBurned, damageNormal * 0.75);
  EXPECT_GT(damageBurned, damageNormal * 0.25);

  // Check self-damage from Burn
  auto getAvgSelfDamage = [](const PossibleEnvironments& results, int teamIdx) {
      double totalDamage = 0;
      double totalProb = 0;
      for (size_t i = 0; i < results.size(); ++i) {
          auto res = results.at(i);
          double prob = res.getProbability().to_double();
          int missingHP = res.getEnv().getTeam(teamIdx).teammate(0).getMissingHP();
          totalDamage += missingHP * prob;
          totalProb += prob;
      }
      return totalProb > 0 ? totalDamage / totalProb : 0.0;
  };

  int maxHP = burnedState.getTeam(TEAM_A).teammate(0).nv().getMaxHP();

  // Total missing HP in result_burned should include damage from setup turn AND current turn.
  double selfDamageBurnedTotal = getAvgSelfDamage(result_burned, TEAM_A);

  std::cout << "Self Damage Burned Total (End of Turn 2): " << selfDamageBurnedTotal << std::endl;

  double expectedDamagePerTurn = maxHP / 8.0;

  // Check that we took roughly 2 turns of damage
  EXPECT_NEAR(selfDamageBurnedTotal, expectedDamagePerTurn * 2, 2.0);

  // Also verify that damageNormal (unburned) has 0 self damage
  double selfDamageNormal = getAvgSelfDamage(result_normal, TEAM_A);
  EXPECT_EQ(selfDamageNormal, 0.0);
}
