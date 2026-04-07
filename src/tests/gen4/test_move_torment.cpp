#include "engine_test.hpp"

class TormentTest : public Gen4EngineTest {
 protected:
  void SetUp() override {
    Gen4EngineTest::SetUp();
    auto team_a = TeamNonVolatile().addPokemon(
        PokemonNonVolatile()
            .setBase(pokedex_->pokemon("alakazam"))
            .addMove(pokedex_->move("torment"))   // 0
            .addMove(pokedex_->move("psychic"))    // 1
            .setLevel(100));

    auto team_b = TeamNonVolatile()
        .addPokemon(PokemonNonVolatile()
          .setBase(pokedex_->pokemon("blissey"))
          .addMove(pokedex_->move("seismic toss")) // 0
          .addMove(pokedex_->move("softboiled"))   // 1
          .setLevel(100));

    environment_nv = EnvironmentNonvolatile(team_a, team_b, true);
    engine_->setEnvironment(environment_nv);
  }
};

TEST_F(TormentTest, AppliesEffect) {
  // Alakazam uses Torment. Blissey uses Seismic Toss.
  auto initialState = engine_->initialState();
  auto results = engine_->updateState(initialState, Action::move(0), Action::move(0));
  auto env = results.where1().getEnv();

  // Blissey should be tormented.
  EXPECT_TRUE(env.teammate(1, 0).status().cTeammate.torment);
}

TEST_F(TormentTest, PreventsConsecutiveMoves) {
  auto initialState = engine_->initialState();
  // Turn 1: Alakazam uses Torment. Blissey uses Seismic Toss (Move 0).
  auto results1 = engine_->updateState(initialState, Action::move(0), Action::move(0));
  auto state1 = results1.where1();

  EXPECT_TRUE(state1.teammate(1, 0).status().cTeammate.torment);
  // Last action should be Seismic Toss (Move 0). iLastAction = 0 + 1 = 1.
  EXPECT_EQ(state1.teammate(1, 0).status().cTeammate.iLastAction, 1);

  // Turn 2: Blissey tries to use Seismic Toss (Move 0). Should be invalid.
  EXPECT_FALSE(engine_->isValidAction(state1, Actor(TEAM_B, 0), Action::move(0)));

  // Blissey tries to use Softboiled (Move 1). Should be valid.
  EXPECT_TRUE(
      engine_->isValidAction(state1, Actor(TEAM_B, 0), Action::moveAlly(1, 0)));

  // Turn 2 execution: Blissey uses Softboiled.
  auto results2 =
      engine_->updateState(state1, Action::move(1), Action::moveAlly(1, 0));
  auto state2 = results2.where1();

  // Last action should be Softboiled (Move 1). iLastAction = 1 + 1 = 2.
  EXPECT_EQ(state2.teammate(1, 0).status().cTeammate.iLastAction, 2);

  // Turn 3: Blissey tries to use Seismic Toss (Move 0). Should be valid now.
  EXPECT_TRUE(engine_->isValidAction(state2, Actor(TEAM_B, 0), Action::move(0)));
  // Blissey tries to use Softboiled (Move 1). Should be invalid.
  EXPECT_FALSE(
      engine_->isValidAction(state2, Actor(TEAM_B, 0), Action::moveAlly(1, 0)));
}

TEST_F(TormentTest, FailsIfAlreadyTormented) {
  auto initialState = engine_->initialState();
  // Turn 1: Alakazam uses Torment.
  auto results1 = engine_->updateState(initialState, Action::move(0), Action::move(0));
  auto state1 = results1.where1();
  EXPECT_TRUE(state1.teammate(1, 0).status().cTeammate.torment);

  // Turn 2: Alakazam uses Torment again.
  auto results2 =
      engine_->updateState(state1, Action::move(0), Action::moveAlly(1, 0));
  auto state2 = results2.where1();
  EXPECT_TRUE(state2.teammate(1, 0).status().cTeammate.torment);
}

TEST_F(TormentTest, ClearsOnSwitch) {
    auto team_a = TeamNonVolatile().addPokemon(
        PokemonNonVolatile()
            .setBase(pokedex_->pokemon("alakazam"))
            .addMove(pokedex_->move("torment"))   // 0
            .addMove(pokedex_->move("psychic"))    // 1
            .setLevel(100));

    auto team_b_switch = TeamNonVolatile()
        .addPokemon(PokemonNonVolatile()
          .setBase(pokedex_->pokemon("blissey"))
          .addMove(pokedex_->move("seismic toss"))
          .addMove(pokedex_->move("softboiled"))
          .setLevel(100))
        .addPokemon(PokemonNonVolatile()
          .setBase(pokedex_->pokemon("chansey")) // Chansey is similar
          .addMove(pokedex_->move("seismic toss"))
          .setLevel(100));

    EnvironmentNonvolatile env_nv(team_a, team_b_switch, true);
    engine_->setEnvironment(env_nv);
    auto initialState = engine_->initialState();

    // Turn 1: Alakazam uses Torment. Blissey uses Seismic Toss.
    auto results1 = engine_->updateState(initialState, Action::move(0), Action::move(0));
    auto state1 = results1.where1();
    EXPECT_TRUE(state1.teammate(1, 0).status().cTeammate.torment);

    // Turn 2: Blissey switches to Chansey.
    auto results2 = engine_->updateState(state1, Action::wait(), Action::swap(1));
    auto state2 = results2.where1();

    // Chansey should NOT have torment.
    EXPECT_FALSE(state2.teammate(1, 0).status().cTeammate.torment);

    // Turn 3: Switch back to Blissey.
    auto results3 = engine_->updateState(state2, Action::wait(), Action::swap(0));
    auto state3 = results3.where1();

    // Blissey should have torment cleared.
    EXPECT_FALSE(state3.teammate(1, 0).status().cTeammate.torment);
}
