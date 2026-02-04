#include "engine_test.hpp"

class SturdyTest : public Gen4EngineTest {
 protected:
  void SetUp() override {
    Gen4EngineTest::SetUp();

    // Team A: Level 100 Walrein with Sheer Cold and Fissure
    // Walrein is Ice/Water, so it gets STAB on Sheer Cold (doesn't matter for OHKO)
    team_a = TeamNonVolatile()
        .addPokemon(PokemonNonVolatile()
          .setBase(pokedex_->pokemon("walrein"))
          .addMove(pokedex_->move("sheer cold"))
          .addMove(pokedex_->move("fissure"))
          .setLevel(100));

    // Team B with Sturdy: Level 1 Geodude
    // Geodude is Rock/Ground
    team_b_sturdy = TeamNonVolatile()
        .addPokemon(PokemonNonVolatile()
          .setBase(pokedex_->pokemon("geodude"))
          .setAbility(pokedex_->ability("sturdy"))
          .addMove(pokedex_->move("tackle"))
          .setLevel(1));

    // Team B without Sturdy: Level 1 Shinx with Intimidate
    team_b_no_sturdy = TeamNonVolatile()
        .addPokemon(PokemonNonVolatile()
          .setBase(pokedex_->pokemon("shinx"))
          .setAbility(pokedex_->ability("intimidate"))
          .addMove(pokedex_->move("tackle"))
          .setLevel(1));

    // Team C: Level 100 Pinsir with Guillotine
    team_c = TeamNonVolatile()
        .addPokemon(PokemonNonVolatile()
          .setBase(pokedex_->pokemon("pinsir"))
          .addMove(pokedex_->move("guillotine"))
          .setLevel(100));
  }

  TeamNonVolatile team_a;
  TeamNonVolatile team_b_sturdy;
  TeamNonVolatile team_b_no_sturdy;
  TeamNonVolatile team_c;
};

TEST_F(SturdyTest, SturdyBlocksSheerCold) {
  engine_->setEnvironment(EnvironmentNonvolatile(team_a, team_b_sturdy, true));

  // Walrein uses Sheer Cold (move 0) on Geodude
  auto turn1 = engine_->updateState(engine_->initialState(), Action::move(0), Action::wait());

  auto geodude = turn1.where1().getTeam(TEAM_B).getPKV();

  // Should take no damage (HP equal to Max HP) because Sturdy blocks OHKO
  EXPECT_EQ(geodude.getHP(), geodude.nv().getMaxHP());
}

TEST_F(SturdyTest, SturdyBlocksFissure) {
  engine_->setEnvironment(EnvironmentNonvolatile(team_a, team_b_sturdy, true));

  // Walrein uses Fissure (move 1) on Geodude
  auto turn1 = engine_->updateState(engine_->initialState(), Action::move(1), Action::wait());

  auto geodude = turn1.where1().getTeam(TEAM_B).getPKV();

  // Should take no damage (HP equal to Max HP) because Sturdy blocks OHKO
  EXPECT_EQ(geodude.getHP(), geodude.nv().getMaxHP());
}

TEST_F(SturdyTest, SheerColdWorksWithoutSturdy) {
  engine_->setEnvironment(EnvironmentNonvolatile(team_a, team_b_no_sturdy, true));

  // Walrein uses Sheer Cold (move 0) on Shinx (Intimidate)
  // Accuracy check: (100 - 1) + 30 = 129% -> Guaranteed hit
  auto turn1 = engine_->updateState(engine_->initialState(), Action::move(0), Action::wait());

  auto shinx = turn1.where1().getTeam(TEAM_B).getPKV();

  // Should be KO'd
  EXPECT_EQ(shinx.getHP(), 0);
  EXPECT_FALSE(shinx.isAlive());
}

TEST_F(SturdyTest, SturdyBlocksGuillotine) {
  engine_->setEnvironment(EnvironmentNonvolatile(team_c, team_b_sturdy, true));

  // Pinsir uses Guillotine (move 0) on Geodude
  auto turn1 = engine_->updateState(engine_->initialState(), Action::move(0), Action::wait());

  auto geodude = turn1.where1().getTeam(TEAM_B).getPKV();

  // Should take no damage (HP equal to Max HP) because Sturdy blocks OHKO
  EXPECT_EQ(geodude.getHP(), geodude.nv().getMaxHP());
}

TEST_F(SturdyTest, GuillotineWorksWithoutSturdy) {
  engine_->setEnvironment(EnvironmentNonvolatile(team_c, team_b_no_sturdy, true));

  // Pinsir uses Guillotine (move 0) on Shinx (Intimidate)
  // Shinx is Electric (not Ghost).
  auto turn1 = engine_->updateState(engine_->initialState(), Action::move(0), Action::wait());

  auto shinx = turn1.where1().getTeam(TEAM_B).getPKV();

  // Should be KO'd
  EXPECT_EQ(shinx.getHP(), 0);
  EXPECT_FALSE(shinx.isAlive());
}

TEST_F(SturdyTest, FissureWorksWithoutSturdy) {
  engine_->setEnvironment(EnvironmentNonvolatile(team_a, team_b_no_sturdy, true));

  // Walrein uses Fissure (move 1) on Shinx (Intimidate)
  // Shinx is grounded (Electric), so Fissure hits.
  // Accuracy check: (100 - 1) + 30 = 129% -> Guaranteed hit
  auto turn1 = engine_->updateState(engine_->initialState(), Action::move(1), Action::wait());

  auto shinx = turn1.where1().getTeam(TEAM_B).getPKV();

  // Should be KO'd
  EXPECT_EQ(shinx.getHP(), 0);
  EXPECT_FALSE(shinx.isAlive());
}
