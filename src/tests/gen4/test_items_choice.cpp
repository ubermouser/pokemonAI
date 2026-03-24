#include "engine_test.hpp"

class ChoiceItemsTest : public Gen4EngineTest {
 protected:
  void SetUp() override {
    Gen4EngineTest::SetUp();

    auto team_1 = TeamNonVolatile()
        .addPokemon(PokemonNonVolatile()
          .setBase(pokedex_->pokemon("scizor"))
          .addMove(pokedex_->move("bullet punch")) // increased by choice band
          .addMove(pokedex_->move("swift")) // not increased
          .setInitialItem(pokedex_->item("choice band"))
          .setLevel(100))
        .addPokemon(PokemonNonVolatile()
          .setBase(pokedex_->pokemon("azelf"))
          .addMove(pokedex_->move("swift")) // increased by choice scarf
          .addMove(pokedex_->move("fire punch")) // not increased
          .setInitialItem(pokedex_->item("choice specs"))
          .setLevel(100))
        .addPokemon(PokemonNonVolatile()
          .setBase(pokedex_->pokemon("flygon"))
          .addMove(pokedex_->move("draco meteor"))
          .setInitialItem(pokedex_->item("choice scarf"))
          .setLevel(100));
    auto team_2 = team_1;
    team_2.teammate(0).setNoInitialItem();
    team_2.teammate(1).setNoInitialItem();
    team_2.teammate(2).setNoInitialItem();
    auto environment = EnvironmentNonvolatile(team_1, team_2, true);
    engine_->setEnvironment(environment);

    bulletpunch_cb = engine_->updateState(engine_->initialState(), Action::move(0), Action::move(0));
    swift_cb = engine_->updateState(engine_->initialState(), Action::move(1), Action::move(1));

    azelf_pair = engine_->updateState(engine_->initialState(), Action::swap(1), Action::swap(1));
    swift_cs = engine_->updateState(
        azelf_pair.where1(), Action::move(0), Action::move(0));
    firepunch_cs = engine_->updateState(
        azelf_pair.where1(), Action::move(1), Action::move(1));

    flygon_pair = engine_->updateState(engine_->initialState(), Action::swap(2), Action::swap(2));
    dracometeor_cs = engine_->updateState(
        flygon_pair.where1(), Action::move(0), Action::move(0));
    dracometeor_none = engine_->updateState(
        flygon_pair.where1(), Action::wait(), Action::move(0));
  }

  PossibleEnvironments bulletpunch_cb;
  PossibleEnvironments swift_cb;
  PossibleEnvironments azelf_pair;
  PossibleEnvironments swift_cs;
  PossibleEnvironments firepunch_cs;
  PossibleEnvironments flygon_pair;
  PossibleEnvironments dracometeor_cs;
  PossibleEnvironments dracometeor_none;
};

TEST_F(ChoiceItemsTest, LockedIntoMove) {
  // other moves are locked out after using a choice move:
  EXPECT_TRUE(
      engine_->isValidAction(bulletpunch_cb.where1().getEnv(), Actor(TEAM_A, 0), Action::move(0)));
  EXPECT_FALSE(
      engine_->isValidAction(bulletpunch_cb.where1().getEnv(), Actor(TEAM_A, 0), Action::move(1)));
}

TEST_F(ChoiceItemsTest, StruggleWhenNoPP) {
  // when all PP have been used, only struggle is available:
  auto noPPState = bulletpunch_cb.where1Hit(0);
  noPPState.getTeam(0).getPKV().getMV(0).setPP(0);
  EXPECT_FALSE(engine_->isValidAction(noPPState.getEnv(), Actor(TEAM_A, 0), Action::move(0))); // locked due to PP
  EXPECT_FALSE(engine_->isValidAction(noPPState.getEnv(), Actor(TEAM_A, 0), Action::move(1))); // locked due to Choice
  EXPECT_TRUE(engine_->isValidAction(noPPState.getEnv(), Actor(TEAM_A, 0), Action::struggle()));
}

TEST_F(ChoiceItemsTest, ChoiceBandBonus) {
  // physical attack with choice band deals additional damage:
  EXPECT_GT(
      bulletpunch_cb.where1().teammate(0, 0).getHP(),
      bulletpunch_cb.where1().teammate(1, 0).getHP());
}

TEST_F(ChoiceItemsTest, ChoiceBandNoSpecialBonus) {
  // special attack with choice band does no additional damage:
  EXPECT_EQ(
      swift_cb.where1().teammate(0, 0).getHP(),
      swift_cb.where1().teammate(1, 0).getHP());
}

TEST_F(ChoiceItemsTest, ChoiceSpecsBonus) {
  // special attack with choice specs deals additional damage:
  EXPECT_GT(
      swift_cs.where1().teammate(0, 1).getHP(),
      swift_cs.where1().teammate(1, 1).getHP());
}

TEST_F(ChoiceItemsTest, ChoiceSpecsNoPhysicalBonus) {
  // physical attack with choice specs does no additional damage
  // Find a state where both teams hit without secondary effects (e.g., burn)
  auto noStatusState = firepunch_cs.where1HitNoStatus(0);

  EXPECT_EQ(
      noStatusState.teammate(0, 1).getHP(),
      noStatusState.teammate(1, 1).getHP());
}

TEST_F(ChoiceItemsTest, ChoiceScarfSpeed) {
  // speed boost with 1-hit KO moves before enemy can deal damage with choice scarf:
  EXPECT_EQ(dracometeor_cs.where1().teammate(0, 2).getPercentHP(), 1.);
  EXPECT_GE(
      dracometeor_cs.where1().getProbability().to_double(),
      0.89);  // enemy never moves
  EXPECT_EQ(dracometeor_cs.where1().teammate(1, 2).getHP(), 0);
  EXPECT_EQ(dracometeor_none.where1().teammate(0, 2).getHP(), 0);
}
