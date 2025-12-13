#include "engine_test.hpp"


class PaybackTest : public EngineTest {
protected:
  void SetUp() override {
    EngineTest::SetUp();

    auto team = TeamNonVolatile()
        .addPokemon(PokemonNonVolatile()
          .setBase(pokedex_->pokemon("machamp"))
          .addMove(pokedex_->move("payback"))
          .addMove(pokedex_->move("bullet punch"))
          .setIV(FV_SPEED, 30) // always faster
          .setLevel(100))
        .addPokemon(PokemonNonVolatile()
          .setBase(pokedex_->pokemon("machoke"))
          .addMove(pokedex_->move("leer"))
          .addMove(pokedex_->move("bullet punch"))
          .setIV(FV_DEFENSE, 20) // match Machamp's base defense
          .setLevel(100));
    environment_nv = EnvironmentNonvolatile(team, team, true);
    engine_->setEnvironment(environment_nv);

    setup_payback = engine_->updateState(engine_->initialState(), Action::wait(), Action::swap(1));

    // we need this to be true so the moving first and enemy switching tests
    // return the same damage taken
    EXPECT_EQ(
      environment_nv.getTeam(0).teammate(0).getFV_base(FV_DEFENSE),
      environment_nv.getTeam(0).teammate(1).getFV_base(FV_DEFENSE));
  }

  PossibleEnvironments setup_payback;
};


TEST_F(PaybackTest, normal_power_when_moving_first) {
  auto payback_normal_power = engine_->updateState(
    setup_payback.at(0), Action::move(0), Action::move(0));

  EXPECT_EQ(payback_normal_power.at(0).getEnv().getTeam(1).getPKV().getMissingHP(), 31);
}


TEST_F(PaybackTest, normal_power_when_enemy_switching) {
  // NOTE: normal damage on switching is technically gen-V+ behavior, but
  // we test it here as smogon uses this behavior for Gen-IV
  auto payback_normal_power = engine_->updateState(
    setup_payback.at(0), Action::move(0), Action::swap(0));

  EXPECT_EQ(payback_normal_power.at(0).getEnv().getTeam(1).getPKV().getMissingHP(), 31);
}


TEST_F(PaybackTest, double_power_when_moving_second) {
  // machoke uses a priority move, machamp will move second
  auto payback_doubled_power = engine_->updateState(
    setup_payback.at(0), Action::move(0), Action::move(1));

  EXPECT_EQ(payback_doubled_power.at(0).getEnv().getTeam(1).getPKV().getMissingHP(), 63);
}
