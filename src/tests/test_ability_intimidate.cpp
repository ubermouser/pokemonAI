#include "engine_test.hpp"


class IntimidateTest : public Gen4EngineTest {
 protected:
  void SetUp() override {
    Gen4EngineTest::SetUp();

    // Team A: Charmander (Lead), Staraptor (Intimidate)
    auto team_a = TeamNonVolatile()
        .addPokemon(PokemonNonVolatile()
          .setBase(pokedex_->pokemon("charmander"))
          .addMove(pokedex_->move("growl"))
          .setLevel(100))
        .addPokemon(PokemonNonVolatile()
          .setBase(pokedex_->pokemon("staraptor"))
          .setAbility(pokedex_->ability("intimidate"))
          .addMove(pokedex_->move("tackle"))
          .setLevel(100));

    // Team B: Charmander (Normal), Metagross (Clear Body)
    auto team_b = TeamNonVolatile()
        .addPokemon(PokemonNonVolatile()
          .setBase(pokedex_->pokemon("charmander"))
          .setAbility(pokedex_->ability("blaze"))
          .addMove(pokedex_->move("growl"))
          .setLevel(100))
        .addPokemon(PokemonNonVolatile()
          .setBase(pokedex_->pokemon("metagross"))
          .setAbility(pokedex_->ability("clear body"))
          .addMove(pokedex_->move("zen headbutt"))
          .setLevel(100));

    engine_->setEnvironment(EnvironmentNonvolatile(team_a, team_b, true));

    // State 1: A:Charmander vs B:Charmander
    // We execute a Wait/Wait turn to wrap this in PossibleEnvironments for consistency
    setup_vs_normal = engine_->updateState(engine_->initialState(), Action::wait(), Action::wait());

    // State 2: A:Charmander vs B:Metagross
    // Uses setup_vs_normal.where1()
    setup_vs_clearbody = engine_->updateState(
        setup_vs_normal.where1(), Action::wait(), Action::swap(1));
  }

  PossibleEnvironments setup_vs_normal;
  PossibleEnvironments setup_vs_clearbody;
};


TEST_F(IntimidateTest, IntimidateVsNormal) {
  // Scenario: Team A switches in Staraptor against Team B's Charmander

  // A switches to Staraptor (index 1)
  auto turn1 = engine_->updateState(
      setup_vs_normal.where1(), Action::swap(1), Action::wait());

  // Verify Charmander (Team B) Attack is -1
  auto charmander_opp = turn1.where1().getTeam(TEAM_B).getPKV();
  EXPECT_EQ(charmander_opp.getBoost(FV_ATTACK), -1);
}


TEST_F(IntimidateTest, IntimidateVsClearBody) {
  // Scenario: Team A switches in Staraptor against Team B's Metagross (Clear Body)

  // A switches to Staraptor (index 1)
  auto turn1 = engine_->updateState(
      setup_vs_clearbody.where1(), Action::swap(1), Action::wait());

  // Verify Metagross (Team B) Attack is 0 (immune)
  auto metagross_opp = turn1.where1().getTeam(TEAM_B).getPKV();
  EXPECT_EQ(metagross_opp.getBoost(FV_ATTACK), 0);
}

// TODO(@drendleman) - intimidate needs to trigger on match-start as well!
