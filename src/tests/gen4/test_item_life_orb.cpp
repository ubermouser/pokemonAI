#include "engine_test.hpp"

class LifeOrbTest : public Gen4EngineTest {
 protected:
  void SetUp() override {
    Gen4EngineTest::SetUp();

    auto team_1 = TeamNonVolatile().addPokemon(
        PokemonNonVolatile()
            .setBase(pokedex_->pokemon("dusknoir"))
            .addMove(pokedex_->move("pain split"))
            .addMove(pokedex_->move("shadow punch"))
            .addMove(pokedex_->move("will-o-wisp"))
            .addMove(pokedex_->move("calm mind"))
            .setInitialItem(pokedex_->item("life orb"))
            .setLevel(100));
    auto team_2 = team_1;
    team_2.teammate(0).setNoInitialItem();
    environment_nv = EnvironmentNonvolatile(team_1, team_2, true);
    engine_->setEnvironment(environment_nv);

    sp_lifeorb = engine_->updateState(
        engine_->initialState(), Action::move(1), Action::wait());
    sp_noitem = engine_->updateState(
        engine_->initialState(), Action::wait(), Action::move(1));
    will_o_wisp = engine_->updateState(
        engine_->initialState(), Action::move(2), Action::wait());
    split_pain = engine_->updateState(
        sp_lifeorb.where1(), Action::move(0), Action::wait());
    calm_mind = engine_->updateState(
        engine_->initialState(), Action::moveAlly(3, 0), Action::wait());
  }

  PossibleEnvironments sp_lifeorb;
  PossibleEnvironments sp_noitem;
  PossibleEnvironments will_o_wisp;
  PossibleEnvironments split_pain;
  PossibleEnvironments calm_mind;
};

TEST_F(LifeOrbTest, LifeOrbSubtractsHP) {
  // attacking move: life orb subtracts 10%, damage is increased by 30%
  EXPECT_EQ(sp_lifeorb.where1().teammate(0, 0).getHP(),
            180);  // 90%
  EXPECT_EQ(sp_lifeorb.where1().teammate(1, 0).getHP(),
            62);  // 31%
  EXPECT_LT(
      sp_lifeorb.where1().teammate(1, 0).getHP(),
      sp_noitem.where1().teammate(0, 0).getHP());
}


TEST_F(LifeOrbTest, NoEffectOnSpecialMove) {
  EXPECT_EQ(
      split_pain.where1().teammate(0, 0).getHP(),
      split_pain.where1().teammate(1, 0).getHP());
}


TEST_F(LifeOrbTest, NoEffectOnStatusMove) {
  EXPECT_EQ(will_o_wisp.where1().teammate(0, 0).getPercentHP(), 1.);
}


TEST_F(LifeOrbTest, NoEffectTargetingFriendlyTeam) {
  EXPECT_EQ(calm_mind.where1().teammate(0, 0).getPercentHP(), 1.);
}