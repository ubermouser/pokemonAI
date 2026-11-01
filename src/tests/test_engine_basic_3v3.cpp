#include "mock_engine_test.hpp"


class BasicEngine3v3Test : public MockEngineTest {
 protected:
  void SetUp() override {
    MockEngineTest::SetUp();
    plugin_calls.fill(0);

    // clang-format off
    auto makeTeam = [&](uint8_t iv0, uint8_t iv1, uint8_t iv2) {
      return TeamNonVolatile()
          .addPokemon(PokemonNonVolatile()
              .setBase(pokedex_->pokemon("test_pokemon"))
              .setIV(FV_SPEED, iv0)
              .addMove(pokedex_->move("move_any_adjacent"))
              .addMove(pokedex_->move("move_all_adjacent_enemy"))
              .addMove(pokedex_->move("move_all_adjacent"))
              .setLevel(100))
          .addPokemon(PokemonNonVolatile()
              .setBase(pokedex_->pokemon("test_pokemon2"))
              .setIV(FV_SPEED, iv1)
              .addMove(pokedex_->move("move_any_adjacent"))
              .addMove(pokedex_->move("move_all_adjacent_enemy"))
              .addMove(pokedex_->move("move_all_adjacent"))
              .setLevel(100))
          .addPokemon(PokemonNonVolatile()
              .setBase(pokedex_->pokemon("test_pokemon3"))
              .setIV(FV_SPEED, iv2)
              .addMove(pokedex_->move("move_any_adjacent"))
              .addMove(pokedex_->move("move_all_adjacent_enemy"))
              .addMove(pokedex_->move("move_all_adjacent"))
              .setLevel(100));
    };
    // clang-format on

    environment_nv =
        EnvironmentNonvolatile(makeTeam(30, 20, 10), makeTeam(25, 15, 5), true);
    engine_->setNumActivePokemon(3);
    engine_->setEnvironment(environment_nv);
  }
};


TEST_F(BasicEngine3v3Test, HighEngineAccuracy_SixSpreadMoves_Random) {
  spdlog::set_level(spdlog::level::warn);
  srand(42);
  engine_->setAccuracy(16);
  engine_->setMaxNumStates(128);
  engine_->setStateSelectMethod(PkCU::StateSelectMethod::RANDOM);

  // clang-format off
  PossibleEnvironments result = engine_->updateState(
      engine_->initialState(),
      {{Actor(TEAM_A, 0), Action::moveAdjacent(2)},
       {Actor(TEAM_A, 1), Action::moveAdjacent(2)},
       {Actor(TEAM_A, 2), Action::moveAdjacent(2)}},
      {{Actor(TEAM_B, 0), Action::moveAdjacent(2)},
       {Actor(TEAM_B, 1), Action::moveAdjacent(2)},
       {Actor(TEAM_B, 2), Action::moveAdjacent(2)}}
  );
  // clang-format on

  EXPECT_GT(result.size(), 0);
  EXPECT_LE(result.size(), 128);
  EXPECT_TRUE(result.saneProbabilities());
}


TEST_F(BasicEngine3v3Test, HighEngineAccuracy_SixSpreadMoves_MostLikely) {
  spdlog::set_level(spdlog::level::warn);
  engine_->setAccuracy(16);
  engine_->setMaxNumStates(128);
  engine_->setStateSelectMethod(PkCU::StateSelectMethod::MOST_LIKELY);

  // clang-format off
  PossibleEnvironments result = engine_->updateState(
      engine_->initialState(),
      {{Actor(TEAM_A, 0), Action::moveAdjacent(2)},
       {Actor(TEAM_A, 1), Action::moveAdjacent(2)},
       {Actor(TEAM_A, 2), Action::moveAdjacent(2)}},
      {{Actor(TEAM_B, 0), Action::moveAdjacent(2)},
       {Actor(TEAM_B, 1), Action::moveAdjacent(2)},
       {Actor(TEAM_B, 2), Action::moveAdjacent(2)}}
  );
  // clang-format on

  EXPECT_GT(result.size(), 0);
  EXPECT_LE(result.size(), 128);
  EXPECT_TRUE(result.saneProbabilities());
}
