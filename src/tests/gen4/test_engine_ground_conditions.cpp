#include "engine_test.hpp"


class GroundConditionsTest : public Gen4EngineTest {
 protected:
  void SetUp() override {
    Gen4EngineTest::SetUp();

    // clang-format off
    auto team = TeamNonVolatile()
        .addPokemon(PokemonNonVolatile() // index 0: has rapid spin (Active)
          .setBase(pokedex_->pokemon("forretress"))
          .addMove(pokedex_->move("stealth rock"))
          .addMove(pokedex_->move("spikes"))
          .addMove(pokedex_->move("toxic spikes"))
          .addMove(pokedex_->move("rapid spin"))
          .setLevel(100))
        .addPokemon(PokemonNonVolatile() // index 1: normal pokemon
          .setBase(pokedex_->pokemon("rattata"))
          .setLevel(100))
        .addPokemon(PokemonNonVolatile() // index 2: has levitate
          .setBase(pokedex_->pokemon("azelf"))
          .setAbility(pokedex_->ability("levitate"))
          .setLevel(100))
        .addPokemon(PokemonNonVolatile() // index 3: is flying type
          .setBase(pokedex_->pokemon("pidgey"))
          .setLevel(100))
        .initialize();
    // clang-format on

    env_nv = std::make_shared<EnvironmentNonvolatile>(team, team, true);
    engine_->setEnvironment(env_nv);
  }

  PossibleEnvironments setupStealthRock() {
    return engine_->updateState(
        engine_->initialState(), Action::move(0), Action::wait());
  }

  PossibleEnvironments setupSpikes() {
    return engine_->updateState(
        engine_->initialState(), Action::move(1), Action::wait());
  }

  PossibleEnvironments setupToxicSpikes() {
    return engine_->updateState(
        engine_->initialState(), Action::move(2), Action::wait());
  }

  std::shared_ptr<const EnvironmentNonvolatile> env_nv;
};


TEST_F(GroundConditionsTest, RapidSpinRemovesSpikes) {
  auto spikes = setupSpikes();
  auto spikes_removed =
      engine_->updateState(spikes.where1(), Action::wait(), Action::move(3));
  auto removed_vs_spikes = engine_->updateState(
      spikes_removed.where1(), Action::wait(), Action::swap(1));

  EXPECT_EQ(removed_vs_spikes.where1().teammate(1, 1).getPercentHP(), 1.);
}


TEST_F(GroundConditionsTest, SpikesHurtNormal) {
  auto spikes = setupSpikes();
  auto result =
      engine_->updateState(spikes.where1(), Action::wait(), Action::swap(1));

  EXPECT_NEAR(result.where1().teammate(1, 1).getPercentHP(), 0.875, 0.005);
}


TEST_F(GroundConditionsTest, ToxicSpikesHurtAndPoisonNormal) {
  auto toxic_spikes = setupToxicSpikes();
  auto result = engine_->updateState(
      toxic_spikes.where1(), Action::wait(), Action::swap(1));

  EXPECT_NEAR(result.where1().teammate(1, 1).getPercentHP(), 0.875, 0.005);
  EXPECT_EQ(result.where1().teammate(1, 1).getStatusAilment(), AIL_NV_POISON);
}


TEST_F(GroundConditionsTest, SpikesDoNotHurtLevitate) {
  auto spikes = setupSpikes();
  auto result =
      engine_->updateState(spikes.where1(), Action::wait(), Action::swap(2));

  EXPECT_EQ(result.where1().teammate(1, 2).getPercentHP(), 1.);
}


TEST_F(GroundConditionsTest, ToxicSpikesDoNotHurtLevitate) {
  auto toxic_spikes = setupToxicSpikes();
  auto result = engine_->updateState(
      toxic_spikes.where1(), Action::wait(), Action::swap(2));

  EXPECT_EQ(result.where1().teammate(1, 2).getPercentHP(), 1.);
}


TEST_F(GroundConditionsTest, StealthRockHurtsLevitate) {
  auto stealth_rock = setupStealthRock();
  auto result = engine_->updateState(
      stealth_rock.where1(), Action::wait(), Action::swap(2));

  EXPECT_NEAR(result.where1().teammate(1, 2).getPercentHP(), 0.875, 0.005);
}


TEST_F(GroundConditionsTest, SpikesDoNotHurtFlying) {
  auto spikes = setupSpikes();
  auto result =
      engine_->updateState(spikes.where1(), Action::wait(), Action::swap(3));

  EXPECT_EQ(result.where1().teammate(1, 3).getPercentHP(), 1.);
}


TEST_F(GroundConditionsTest, StealthRockHurtsFlying) {
  auto stealth_rock = setupStealthRock();
  auto result = engine_->updateState(
      stealth_rock.where1(), Action::wait(), Action::swap(3));

  EXPECT_NEAR(result.where1().teammate(1, 3).getPercentHP(), 0.75, 0.005);
}


TEST_F(GroundConditionsTest, StealthRockReported) {
  auto result = setupStealthRock();
  auto output = StateTransitionPrinter::printString(
      engine_->initialState(), result.where1(), false);

  SCOPED_TRACE(output);
  EXPECT_TRUE(
      output.find("Pointed stones float in the air around forretress") !=
      std::string::npos);
}


TEST_F(GroundConditionsTest, SpikesReported) {
  auto result = setupSpikes();
  auto output = StateTransitionPrinter::printString(
      engine_->initialState(), result.where1(), false);

  SCOPED_TRACE(output);
  EXPECT_TRUE(
      output.find("Spikes were scattered around forretress's feet") !=
      std::string::npos);
}


TEST_F(GroundConditionsTest, ToxicSpikesReported) {
  auto result = setupToxicSpikes();
  auto output = StateTransitionPrinter::printString(
      engine_->initialState(), result.where1(), false);

  SCOPED_TRACE(output);
  EXPECT_TRUE(
      output.find("Toxic spikes were scattered around forretress's feet") !=
      std::string::npos);
}
