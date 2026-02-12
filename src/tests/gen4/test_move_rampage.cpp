#include "engine_test.hpp"

class RampageTest : public Gen4EngineTest {
 protected:
  void runRampageTest(const std::string& move) {
    // clang-format off
    auto team_1 =TeamNonVolatile()
        .addPokemon(PokemonNonVolatile()
          .setBase(pokedex_->pokemon("smeargle"))
          .addMove(pokedex_->move(move))
          .addMove(pokedex_->move("tackle"))
          .setLevel(100))
        .addPokemon(PokemonNonVolatile()
          .setBase(pokedex_->pokemon("pikachu")));
    auto team_2 = TeamNonVolatile()
        .addPokemon(PokemonNonVolatile()
          .setBase(pokedex_->pokemon("metagross"))
          .setLevel(100));
    // clang-format on
    auto environment = EnvironmentNonvolatile(team_1, team_2, true);
    engine_->setEnvironment(environment);

    auto rampage_0 = engine_->updateState(
        engine_->initialState(), Action::move(0), Action::wait());
    auto rampage_1 = engine_->updateState(
        rampage_0.where1(), Action::move(0), Action::wait());
    auto rampage_2 = engine_->updateState(
        rampage_1.where1(), Action::move(0), Action::wait());

    // the pokemon cannot switch out or perform other moves when rampaging:
    EXPECT_TRUE(
        engine_->isValidAction(rampage_0.where1(), Action::move(0), TEAM_A));
    EXPECT_FALSE(
        engine_->isValidAction(rampage_0.where1(), Action::move(1), TEAM_A));
    EXPECT_FALSE(
        engine_->isValidAction(rampage_0.where1(), Action::swap(1), TEAM_A));

    // the pokemon is confused after rampaging:
    EXPECT_EQ(
        rampage_2.where1().teammate(0, 0).status().cTeammate.confused,
        AIL_V_CONFUSED_5T);
    // the pokemon may switch out or perform other moves:
    EXPECT_TRUE(
        engine_->isValidAction(rampage_2.where1(), Action::move(0), TEAM_A));
    EXPECT_TRUE(
        engine_->isValidAction(rampage_2.where1(), Action::move(1), TEAM_A));
    EXPECT_TRUE(
        engine_->isValidAction(rampage_2.where1(), Action::swap(1), TEAM_A));
  }
};

TEST_F(RampageTest, Outrage) { runRampageTest("outrage"); }

TEST_F(RampageTest, PetalDance) { runRampageTest("petal dance"); }

TEST_F(RampageTest, Thrash) { runRampageTest("thrash"); }
