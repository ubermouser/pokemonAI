#include <gtest/gtest.h>

#include <memory>
#include <vector>

#include "pokemonai/engine.h"
#include "pokemonai/pokedex_static.h"
#include "pokemonai/pkCU.h"


class StatusTest : public ::testing::Test {
protected:
  void SetUp() override {
    pokedex_ = std::make_shared<PokedexStatic>();
    engine_ = std::make_shared<PkCU>();
    engine_->setAllowInvalidMoves(true);
  }

  std::shared_ptr<Pokedex> pokedex_;
  std::shared_ptr<PkCU> engine_;
};


TEST_F(StatusTest, BurnEffect) {
  // Setup: Machamp (High Attack, Physical) vs Shuckle (High Def, Low HP)
  // Shuckle survives the hit, allowing end-of-turn effects to run.

  // Team A: Machamp
  auto team_a = TeamNonVolatile()
      .addPokemon(PokemonNonVolatile()
        .setBase(pokedex_->pokemon("machamp"))
        .addMove(pokedex_->move("cross chop")) // Physical move
        .setLevel(100));

  // Team B: Shuckle
  auto team_b = TeamNonVolatile()
      .addPokemon(PokemonNonVolatile()
        .setBase(pokedex_->pokemon("shuckle"))
        .addMove(pokedex_->move("stealth rock")) // Implemented move
        .setLevel(100));

  auto environment = EnvironmentNonvolatile(team_a, team_b, true);
  engine_->setEnvironment(environment);
  auto initialState = engine_->initialState();

  // Create a state where Machamp is burned
  auto mutableStateData = initialState.data();
  mutableStateData.teams[TEAM_A].teammates[0].status_nonvolatile = AIL_NV_BURN;

  ConstEnvironmentVolatile burnedState(initialState.nv(), mutableStateData);

  // Scenario 1: Unburned Machamp attacks
  // Cross Chop: 100 Power, Physical
  auto result_normal = engine_->updateState(
      initialState, Action::move(0), Action::wait());

  // Scenario 2: Burned Machamp attacks
  auto result_burned = engine_->updateState(
      burnedState, Action::move(0), Action::wait());

  // Helper to get average damage dealt to opponent
  auto getAvgDamageDealt = [](const PossibleEnvironments& results, int attackerTeam, int defenderTeam) {
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

  double damageNormal = getAvgDamageDealt(result_normal, TEAM_A, TEAM_B);
  double damageBurned = getAvgDamageDealt(result_burned, TEAM_A, TEAM_B);

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

  double selfDamageBurned = getAvgSelfDamage(result_burned, TEAM_A);

  std::cout << "Self Damage Burned: " << selfDamageBurned << std::endl;

  // Burn deals 1/8 of max HP
  int maxHP = initialState.nv().getTeam(TEAM_A).teammate(0).getMaxHP();
  double expectedBurnDamage = maxHP / 8.0;

  std::cout << "Expected Burn Damage: " << expectedBurnDamage << " (MaxHP: " << maxHP << ")" << std::endl;

  EXPECT_NEAR(selfDamageBurned, expectedBurnDamage, 2.0);
}
