#include "engine_test.hpp"


class InnerFocusTest : public Gen4EngineTest {
 protected:
  void SetUp() override {
    Gen4EngineTest::SetUp();

    // Team A: Dragonite (Active, Inner Focus), Starmie (Reserve, Natural Cure)
    auto team_a = TeamNonVolatile()
        .addPokemon(PokemonNonVolatile()
          .setBase(pokedex_->pokemon("dragonite"))
          .setAbility(pokedex_->ability("inner focus"))
          .addMove(pokedex_->move("dragon claw")) 
          .setLevel(100))
        .addPokemon(PokemonNonVolatile()
          .setBase(pokedex_->pokemon("starmie"))
          .setAbility(pokedex_->ability("natural cure"))
          .addMove(pokedex_->move("recover")) 
          .setLevel(100)); // Level 100 (survives hits)

    // Team B: Jirachi (Serene Grace + Iron Head + Choice Scarf)
    auto team_b = TeamNonVolatile()
        .addPokemon(PokemonNonVolatile()
          .setBase(pokedex_->pokemon("jirachi"))
          .setAbility(pokedex_->ability("serene grace"))
          .setInitialItem(pokedex_->item("choice scarf"))
          .addMove(pokedex_->move("iron head"))
          .setLevel(100)); // Faster due to Scarf

    environment_nv = EnvironmentNonvolatile(team_a, team_b, true);
    engine_->setEnvironment(environment_nv);

    swap_starmie = engine_->updateState(
      engine_->initialState(), Action::swap(1), Action::wait());
  }

  PossibleEnvironments swap_starmie;
};


TEST_F(InnerFocusTest, PreventsFlinch) {
  // Dragonite should not be blocked by flinch from Iron Head
  // Both move: Jirachi (move 0), Dragonite (move 0)
  auto turn1_outcome = engine_->updateState(
    engine_->initialState(), Action::move(0), Action::move(0));

  bool found_blocked = false;
  int blocked_count = 0;
  
  for (size_t i = 0; i < turn1_outcome.size(); ++i) {
    auto env = turn1_outcome.at(i);
    // Check if Dragonite (Team A) was blocked
    if (env.flagsFor(TEAM_A).isBlocked()) {
        found_blocked = true;
        blocked_count++;
    }
  }

  EXPECT_FALSE(found_blocked) << "Dragonite was blocked (flinch?) despite Inner Focus! Count: " << blocked_count;
}


TEST_F(InnerFocusTest, ControlGroupFlinches) {
  bool found_blocked = false;
  auto turn2_outcome = engine_->updateState(
      swap_starmie.where1(), Action::moveAlly(0, 1), Action::move(0));

  for (size_t j = 0; j < turn2_outcome.size(); ++j) {
    auto env_final = turn2_outcome.at(j);
    if (env_final.flagsFor(TEAM_A).isBlocked()) {
        found_blocked = true;
    }
  }

  EXPECT_TRUE(found_blocked) << "Starmie was never blocked! Expected flinch from Iron Head.";
}
