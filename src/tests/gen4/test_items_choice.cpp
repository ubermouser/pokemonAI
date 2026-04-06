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
    env_nv_ = std::make_shared<EnvironmentNonvolatile>(team_1, team_2, true);
    engine_->setEnvironment(env_nv_);

    both_hit = EnvironmentBitfield()
                   .flagsFor(TEAM_A)
                   .setHit()
                   .flagsFor(TEAM_B)
                   .setHit();
  }

  PossibleEnvironments azelfVsAzelf() {
    return engine_->updateState(
        engine_->initialState(), Action::swap(1), Action::swap(1));
  }

  PossibleEnvironments flygonVsFlygon() {
    return engine_->updateState(
        engine_->initialState(), Action::swap(2), Action::swap(2));
  }

  PossibleEnvironments afterBulletPunchCB() {
    return engine_->updateState(
        engine_->initialState(), Action::move(0), Action::move(0));
  }

  PossibleEnvironments afterSwiftCB() {
    return engine_->updateState(
        engine_->initialState(), Action::move(1), Action::move(1));
  }

  PossibleEnvironments afterSwiftCS() {
    auto start = azelfVsAzelf();
    return engine_->updateState(
        start.where1().getEnv(), Action::move(0), Action::move(0));
  }

  PossibleEnvironments afterFirePunchCS() {
    auto start = azelfVsAzelf();
    return engine_->updateState(
        start.where1().getEnv(), Action::move(1), Action::move(1));
  }

  PossibleEnvironments afterDracoMeteorCS() {
    auto start = flygonVsFlygon();
    return engine_->updateState(
        start.where1().getEnv(), Action::move(0), Action::move(0));
  }

  PossibleEnvironments afterDracoMeteorNone() {
    auto start = flygonVsFlygon();
    return engine_->updateState(
        start.where1().getEnv(), Action::wait(), Action::move(0));
  }

  std::shared_ptr<const EnvironmentNonvolatile> env_nv_;
  EnvironmentBitfield both_hit;
};


TEST_F(ChoiceItemsTest, LockedIntoMove_CanUseSame) {
  auto result = afterBulletPunchCB();
  EXPECT_TRUE(engine_->isValidAction(
      result.where1().getEnv(), Actor(TEAM_A, 0), Action::move(0)));
}


TEST_F(ChoiceItemsTest, LockedIntoMove_CannotUseOther) {
  auto result = afterBulletPunchCB();
  EXPECT_FALSE(engine_->isValidAction(
      result.where1().getEnv(), Actor(TEAM_A, 0), Action::move(1)));
}


TEST_F(ChoiceItemsTest, StruggleWhenNoPP_LockedSame) {
  auto res = afterBulletPunchCB();
  auto noPPState = res.where1Hit(0);
  noPPState.teammate(0, 0).getMV(0).setPP(0);
  EXPECT_FALSE(engine_->isValidAction(
      noPPState.getEnv(), Actor(TEAM_A, 0), Action::move(0)));
}


TEST_F(ChoiceItemsTest, StruggleWhenNoPP_LockedOther) {
  auto res = afterBulletPunchCB();
  auto noPPState = res.where1Hit(0);
  noPPState.teammate(0, 0).getMV(0).setPP(0);
  EXPECT_FALSE(engine_->isValidAction(
      noPPState.getEnv(), Actor(TEAM_A, 0), Action::move(1)));
}


TEST_F(ChoiceItemsTest, StruggleWhenNoPP_CanStruggle) {
  auto res = afterBulletPunchCB();
  auto noPPState = res.where1Hit(0);
  noPPState.teammate(0, 0).getMV(0).setPP(0);
  EXPECT_TRUE(engine_->isValidAction(noPPState.getEnv(), Actor(TEAM_A, 0), Action::struggle()));
}


TEST_F(ChoiceItemsTest, ChoiceBandBonus) {
  auto result = afterBulletPunchCB();
  EXPECT_GT(
      result.where1().teammate(0, 0).getHP(),
      result.where1().teammate(1, 0).getHP());
}


TEST_F(ChoiceItemsTest, ChoiceBandNoSpecialBonus) {
  auto result = afterSwiftCB();
  EXPECT_EQ(
      result.where1().teammate(0, 0).getHP(),
      result.where1().teammate(1, 0).getHP());
}


TEST_F(ChoiceItemsTest, ChoiceSpecsBonus) {
  auto result = afterSwiftCS();
  EXPECT_GT(
      result.where1().teammate(0, 1).getHP(),
      result.where1().teammate(1, 1).getHP());
}


TEST_F(ChoiceItemsTest, ChoiceSpecsNoPhysicalBonus) {
  auto res = afterFirePunchCS();
  auto noStatusState = res.where1HitNoStatus(0);
  EXPECT_EQ(
      noStatusState.teammate(0, 1).getHP(),
      noStatusState.teammate(1, 1).getHP());
}


TEST_F(ChoiceItemsTest, ChoiceScarfSpeed_FullHP) {
  auto result = afterDracoMeteorCS();
  EXPECT_EQ(result.where1().teammate(0, 2).getPercentHP(), 1.);
}


TEST_F(ChoiceItemsTest, ChoiceScarfSpeed_HighProbability) {
  auto result = afterDracoMeteorCS();
  EXPECT_GE(result.where1().getProbability().to_double(), 0.89);
}


TEST_F(ChoiceItemsTest, ChoiceScarfSpeed_OpponentKOed) {
  auto result = afterDracoMeteorCS();
  EXPECT_EQ(result.where1().teammate(1, 2).getHP(), 0);
}


TEST_F(ChoiceItemsTest, ChoiceScarfSpeed_NoScarfKOed) {
  auto result = afterDracoMeteorNone();
  EXPECT_EQ(result.where1().teammate(0, 2).getHP(), 0);
}
