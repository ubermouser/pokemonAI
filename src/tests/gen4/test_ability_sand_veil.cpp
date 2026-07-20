#include "engine_test.hpp"


class SandVeilTest : public Gen4EngineTest {
 protected:
  void SetUp() override {
    Gen4EngineTest::SetUp();

    auto team_a = TeamNonVolatile()
        .addPokemon(PokemonNonVolatile()
          .setBase(pokedex_->pokemon("blissey"))
          .addMove(pokedex_->move("sandstorm"))
          .addMove(pokedex_->move("pound"))
          .setLevel(100));

    auto team_b = TeamNonVolatile()
        .addPokemon(PokemonNonVolatile()
          .setBase(pokedex_->pokemon("sandshrew"))
          .setAbility(pokedex_->ability("sand veil"))
          .addMove(pokedex_->move("headbutt"))
          .setLevel(100));

    env_nv = std::make_shared<EnvironmentNonvolatile>(team_a, team_b, true);
    engine_->setEnvironment(env_nv);
  }

  PossibleEnvironments setupNormalTurn1() {
    return engine_->updateState(
        engine_->initialState(), Action::move(1), Action::wait());
  }

  PossibleEnvironments setupSandstormTurn1() {
    return engine_->updateState(
        engine_->initialState(), Action::move(0), Action::wait());
  }

  PossibleEnvironments setupSandstormTurn2() {
    return engine_->updateState(
        setupSandstormTurn1().where1(), Action::move(1), Action::wait());
  }

  std::shared_ptr<EnvironmentNonvolatile> env_nv;
};


TEST_F(SandVeilTest, ReducesAccuracyInSandstorm) {
  auto turn2 = setupSandstormTurn2();

  auto hits = turn2.whereHit(TEAM_A);
  auto misses = turn2.whereMiss(TEAM_A);

  EXPECT_FALSE(hits.empty());
  EXPECT_FALSE(misses.empty());

  FixType hitProb = FixType(0);
  for (const auto& s : hits) {
    hitProb += s.getProbability();
  }

  FixType missProb = FixType(0);
  for (const auto& s : misses) {
    missProb += s.getProbability();
  }

  EXPECT_NEAR(double(hitProb), 0.8, 1e-3);
  EXPECT_NEAR(double(missProb), 0.2, 1e-3);
}


TEST_F(SandVeilTest, DoesNotReduceAccuracyOutsideSandstorm) {
  auto turn1 = setupNormalTurn1();

  auto hits = turn1.whereHit(TEAM_A);
  auto misses = turn1.whereMiss(TEAM_A);

  EXPECT_FALSE(hits.empty());
  EXPECT_TRUE(misses.empty());

  FixType hitProb = FixType(0);
  for (const auto& s : hits) {
    hitProb += s.getProbability();
  }
  EXPECT_NEAR(double(hitProb), 1.0, 1e-3);
}
