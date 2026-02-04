#include "engine_test.hpp"

class PerishSongTest : public Gen4EngineTest {
 protected:
  void SetUp() override {
    Gen4EngineTest::SetUp();

    // Team A: Perish Song user (Gengar) + Switch-in (Pikachu)
    auto team_a_nv = TeamNonVolatile()
        .addPokemon(PokemonNonVolatile()
          .setBase(pokedex_->pokemon("gengar"))
          .addMove(pokedex_->move("perish song"))
          .addMove(pokedex_->move("shadow ball"))
          .setLevel(100))
        .addPokemon(PokemonNonVolatile()
          .setBase(pokedex_->pokemon("pikachu"))
          .addMove(pokedex_->move("thunderbolt"))
          .setLevel(100));

    // Team B: Victim (Snorlax)
    auto team_b_nv = TeamNonVolatile()
        .addPokemon(PokemonNonVolatile()
          .setBase(pokedex_->pokemon("snorlax"))
          .addMove(pokedex_->move("tackle"))
          .setLevel(100));

    environment_nv = EnvironmentNonvolatile(team_a_nv, team_b_nv, true);
    engine_->setEnvironment(environment_nv);
  }
};

TEST_F(PerishSongTest, AppliesToBoth) {
  // Turn 1: Gengar uses Perish Song.
  // Result is Start of Turn 2 state (decremented 3->2).
  auto possible_envs = engine_->updateState(
    engine_->initialState(), Action::move(0), Action::move(0));

  auto env = possible_envs.where1();
  EXPECT_EQ(env.getTeam(0).getPKV().status().cTeammate.perishSong, 2);
  EXPECT_EQ(env.getTeam(1).getPKV().status().cTeammate.perishSong, 2);
}

TEST_F(PerishSongTest, FaintsAfter3Turns) {
  // Turn 1: Use Perish Song. Result 2.
  auto envs1 = engine_->updateState(
    engine_->initialState(), Action::move(0), Action::move(0));
  auto state1 = envs1.where1();
  EXPECT_EQ(state1.getTeam(0).getPKV().status().cTeammate.perishSong, 2);
  EXPECT_GT(state1.getTeam(0).getPKV().getHP(), 0);

  // Turn 2: Decrement to 1.
  auto envs2 = engine_->updateState(
    state1.getEnv(), Action::move(1), Action::move(0));
  auto state2 = envs2.where1();
  EXPECT_EQ(state2.getTeam(0).getPKV().status().cTeammate.perishSong, 1);
  EXPECT_GT(state2.getTeam(0).getPKV().getHP(), 0);

  // Turn 3: Decrement to 0 -> Faint.
  auto envs3 = engine_->updateState(
    state2.getEnv(), Action::move(1), Action::move(0));

  // Both should faint.
  auto fainted_envs = envs3.where([](const ConstEnvironmentPossible& env) {
    return env.getTeam(0).getPKV().getHP() == 0 && env.getTeam(1).getPKV().getHP() == 0;
  });

  ASSERT_FALSE(fainted_envs.empty()) << "Both Pokemon should have fainted";
}

TEST_F(PerishSongTest, SwitchClearsEffect) {
  // Turn 1: Use Perish Song. Result 2.
  auto envs1 = engine_->updateState(
    engine_->initialState(), Action::move(0), Action::move(0));
  auto state1 = envs1.where1();
  EXPECT_EQ(state1.getTeam(0).getPKV().status().cTeammate.perishSong, 2);

  // Turn 2: Switch Gengar out to Pikachu (Index 1).
  // Result: Snorlax 2 (Paused).
  auto envs2 = engine_->updateState(
    state1.getEnv(), Action::swap(1), Action::move(0));
  auto state2 = envs2.where1();

  // Pikachu (now active) should have 0 perishSong.
  EXPECT_EQ(state2.getTeam(0).getPKV().status().cTeammate.perishSong, 0);

  // Snorlax (still active) should have 2.
  EXPECT_EQ(state2.getTeam(1).getPKV().status().cTeammate.perishSong, 2);
}
