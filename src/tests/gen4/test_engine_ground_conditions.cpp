#include "engine_test.hpp"

TEST_F(Gen4EngineTest, GroundConditions) {
  auto team = TeamNonVolatile()
      .addPokemon(PokemonNonVolatile() // has rapid spin
        .setBase(pokedex_->pokemon("forretress"))
        .addMove(pokedex_->move("stealth rock"))
        .addMove(pokedex_->move("spikes"))
        .addMove(pokedex_->move("toxic spikes"))
        .addMove(pokedex_->move("rapid spin"))
        .setLevel(100))
      .addPokemon(PokemonNonVolatile() // normal pokemon
        .setBase(pokedex_->pokemon("rattata"))
        .setLevel(100))
      .addPokemon(PokemonNonVolatile() // has levitate
        .setBase(pokedex_->pokemon("azelf"))
        .setAbility(pokedex_->ability("levitate"))
        .setLevel(100))
      .addPokemon(PokemonNonVolatile() // is flying type
        .setBase(pokedex_->pokemon("pidgey"))
        .setLevel(100))
      .initialize();
  auto environment = EnvironmentNonvolatile(team, team, true);
  engine_->setEnvironment(environment);

  auto stealth_rock = engine_->updateState(engine_->initialState(), Action::move(0), Action::wait());
  auto spikes = engine_->updateState(engine_->initialState(), Action::move(1), Action::wait());
  auto toxic_spikes = engine_->updateState(engine_->initialState(), Action::move(2), Action::wait());

  { // test rapid-spin removal:
    auto spikes_removed =
        engine_->updateState(spikes.where1(), Action::wait(), Action::move(3));
    auto removed_vs_spikes = engine_->updateState(
        spikes_removed.where1(), Action::wait(), Action::swap(1));
    EXPECT_EQ(
        removed_vs_spikes.where1().teammate(1, 1).getPercentHP(),
        1.);  // 100%
  }
  { // test normal harmed vs spikes:
    auto normal_vs_spikes =
        engine_->updateState(spikes.where1(), Action::wait(), Action::swap(1));
    EXPECT_NEAR(
        normal_vs_spikes.where1().teammate(1, 1).getPercentHP(),
        0.875,
        0.005);  // 87.5%
  }
  { // test normal harmed vs toxic spikes:
    auto normal_vs_toxic = engine_->updateState(
        toxic_spikes.where1(), Action::wait(), Action::swap(1));
    EXPECT_NEAR(
        normal_vs_toxic.where1().teammate(1, 1).getPercentHP(),
        0.875,
        0.005);  // 87.5%
    EXPECT_EQ(
        normal_vs_toxic.where1()
            .teammate(1, 1)
            .getStatusAilment(),
        AIL_NV_POISON);  // 87.5%
  }
  { // test levitate unharmed vs spikes:
    auto lev_vs_spikes =
        engine_->updateState(spikes.where1(), Action::wait(), Action::swap(2));
    EXPECT_EQ(
        lev_vs_spikes.where1().teammate(1, 2).getPercentHP(),
        1.);  // 100%
  }
  { // test levitate unharmed vs toxic spikes:
    auto lev_vs_toxic = engine_->updateState(
        toxic_spikes.where1(), Action::wait(), Action::swap(2));
    EXPECT_EQ(
        lev_vs_toxic.where1().teammate(1, 2).getPercentHP(),
        1.);  // 100%
  }
  { // test levitate harmed vs stealth rock:
    auto lev_vs_sr = engine_->updateState(
        stealth_rock.where1(), Action::wait(), Action::swap(2));
    EXPECT_NEAR(
        lev_vs_sr.where1().teammate(1, 2).getPercentHP(),
        0.875,
        0.005);  // 87.5%
  }
  { // test flying unharmed vs spikes:
    auto flying_vs_spikes =
        engine_->updateState(spikes.where1(), Action::wait(), Action::swap(3));
    EXPECT_EQ(
        flying_vs_spikes.where1().teammate(1, 3).getPercentHP(),
        1.);  // 100%
  }
  { // test flying harmed vs stealth rock:
    auto flying_vs_sr = engine_->updateState(
        stealth_rock.where1(), Action::wait(), Action::swap(3));
    EXPECT_NEAR(
        flying_vs_sr.where1().teammate(1, 3).getPercentHP(),
        0.75,
        0.005);  // 75%
  }
}
