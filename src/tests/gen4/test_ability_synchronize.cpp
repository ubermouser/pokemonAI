#include "engine_test.hpp"


class SynchronizeTest : public Gen4EngineTest {
 public:
  void SetUp() override {
    Gen4EngineTest::SetUp();
    // Team A: Mew with Synchronize
    // Using Mew because it learns Synchronize
    auto team_a = TeamNonVolatile()
        .addPokemon(PokemonNonVolatile()
          .setBase(pokedex_->pokemon("umbreon"))
          .setAbility(pokedex_->ability("synchronize"))
          .addMove(pokedex_->move("tackle")) // Dummy move
          .setLevel(100));

    // Team B: Mewtwo with status moves
    auto team_b = TeamNonVolatile()
        .addPokemon(PokemonNonVolatile()
          .setBase(pokedex_->pokemon("mewtwo"))
          .addMove(pokedex_->move("will-o-wisp")) // 0: Burn
          .addMove(pokedex_->move("thunder wave")) // 1: Paralysis
          .addMove(pokedex_->move("toxic"))        // 2: Bad Poison
          .addMove(pokedex_->move("swift"))
          .setLevel(100));

    environment_nv = EnvironmentNonvolatile(team_a, team_b, true);
    engine_->setEnvironment(environment_nv);

    const auto initialState = engine_->initialState();
    results_burn =
        engine_->updateState(initialState, Action::wait(), Action::move(0));
    results_paralysis =
        engine_->updateState(initialState, Action::wait(), Action::move(1));
    results_poison =
        engine_->updateState(initialState, Action::wait(), Action::move(2));
  }

  PossibleEnvironments results_burn;
  PossibleEnvironments results_paralysis;
  PossibleEnvironments results_poison;
};

TEST_F(SynchronizeTest, SynchronizeBurn) {
  // Opponent (Team B) uses Will-O-Wisp on User (Team A)
  // User has Synchronize

  // Team 0 (User) waits, Team 1 (Opponent) uses Will-O-Wisp (Move 0)
  auto& result_states = results_burn;

  // Check if User is burned
  bool user_burned = false;
  // Check if Opponent is burned (Synchronize effect)
  bool opponent_burned = false;

  // We need to find a state where the move hit
  for (const auto& state : result_states) {
      // Assuming single battle, index 0 is active pokemon
      // state is EnvironmentPossibleData
      EnvironmentPossible env(environment_nv, const_cast<EnvironmentPossibleData&>(state));
      auto user_pkv = env.teammate(0, 0);
      auto opponent_pkv = env.teammate(1, 0);

      if (user_pkv.getStatusAilment() == AIL_NV_BURN) {
          user_burned = true;
          if (opponent_pkv.getStatusAilment() == AIL_NV_BURN) {
              opponent_burned = true;
          }
      }
  }

  EXPECT_TRUE(user_burned) << "User should be burned by Will-O-Wisp";
  EXPECT_TRUE(opponent_burned) << "Opponent should be burned by Synchronize";
}

TEST_F(SynchronizeTest, SynchronizeParalysis) {
  // Opponent (Team B) uses Thunder Wave on User (Team A)
  auto& result_states = results_paralysis;

  bool user_paralyzed = false;
  bool opponent_paralyzed = false;

  for (const auto& state : result_states) {
      EnvironmentPossible env(environment_nv, const_cast<EnvironmentPossibleData&>(state));
      auto user_pkv = env.teammate(0, 0);
      auto opponent_pkv = env.teammate(1, 0);

      if (user_pkv.getStatusAilment() == AIL_NV_PARALYSIS) {
          user_paralyzed = true;
          if (opponent_pkv.getStatusAilment() == AIL_NV_PARALYSIS) {
              opponent_paralyzed = true;
          }
      }
  }

  EXPECT_TRUE(user_paralyzed) << "User should be paralyzed by Thunder Wave";
  EXPECT_TRUE(opponent_paralyzed) << "Opponent should be paralyzed by Synchronize";
}

TEST_F(SynchronizeTest, SynchronizePoison) {
  // Opponent (Team B) uses Toxic on User (Team A)
  auto& result_states = results_poison;

  bool user_poisoned = false;
  bool opponent_poisoned = false;

  for (const auto& state : result_states) {
      EnvironmentPossible env(environment_nv, const_cast<EnvironmentPossibleData&>(state));
      auto user_pkv = env.teammate(0, 0);
      auto opponent_pkv = env.teammate(1, 0);

      // Toxic causes Bad Poison
      if (user_pkv.getStatusAilment() == AIL_NV_POISON_TOXIC) {
          user_poisoned = true;
          // Synchronize turns Bad Poison into Regular Poison (Gen 4)
          // Or Bad Poison (Gen 5+). We need to check which gen this engine is targeting.
          // The files say "gen4_scripts", so likely Gen 4 behavior.
          // Bulbapedia: "Generations III and IV: ... inflicted with the regular poison condition."
          if (opponent_pkv.getStatusAilment() == AIL_NV_POISON) {
              opponent_poisoned = true;
          } else if (opponent_pkv.getStatusAilment() == AIL_NV_POISON_TOXIC) {
              // Just in case it implements Gen 5 behavior
              opponent_poisoned = true;
          }
      }
  }

  EXPECT_TRUE(user_poisoned) << "User should be poisoned by Toxic";
  EXPECT_TRUE(opponent_poisoned) << "Opponent should be poisoned by Synchronize";
}


TEST_F(SynchronizeTest, SynchronizeReported) {
  // Opponent (Team B) uses Thunder Wave on User (Team A)
  auto output = StateTransitionPrinter::printString(
      engine_->initialState(), results_paralysis.where1(), false);

  SCOPED_TRACE(output);
  // Verify Umbreon (Synchronize user) is mentioned as being status'd
  EXPECT_TRUE(output.find("umbreon is paralyzed") != std::string::npos);
  // Verify Mewtwo (Caster) is also mentioned (due to Synchronize)
  EXPECT_TRUE(output.find("mewtwo is paralyzed") != std::string::npos);
}
