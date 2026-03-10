#include "engine_test.hpp"

TEST_F(Gen4EngineTest, PrimaryHitAndCrit) {
  auto team = TeamNonVolatile()
      .addPokemon(PokemonNonVolatile()
        .setBase(pokedex_->pokemon("charmander"))
        .addMove(pokedex_->move("cut")));
  auto environment = EnvironmentNonvolatile(team, team, true);
  engine_->setEnvironment(environment);

  PossibleEnvironments result = engine_->updateState(
      engine_->initialState(), Action::move(0), Action::wait());

  // pokemon may move freely when both are alive:
  EXPECT_TRUE(engine_->isValidAction(engine_->initialState(), Action::move(0), TEAM_A));
  EXPECT_EQ(result.size(), 3);
  EXPECT_EQ(result.where1Hit(0).hasHit(0), true);
  EXPECT_EQ(result.where1Miss(0).hasHit(0), false);
  EXPECT_EQ(result.where1Crit(0).hasCrit(0), true);
}


TEST_F(Gen4EngineTest, HighEngineAccuracy) {
  // moves with extremely high numbers of branches might cause stack probability that sums less than 1
  engine_->setAccuracy(16);
  auto team = TeamNonVolatile()
      .addPokemon(PokemonNonVolatile()
        .setBase(pokedex_->pokemon("charmander"))
        .addMove(pokedex_->move("fire blast"))); // move can hit, crit, status, and miss
  auto environment = EnvironmentNonvolatile(team, team, true);
  engine_->setEnvironment(environment);

  PossibleEnvironments result = engine_->updateState(
      engine_->initialState(), Action::move(0), Action::move(0));

  EXPECT_EQ(result.size(), 8450);
  EXPECT_EQ(result.getNumUnique(), 49);
  result.printStates();
}


TEST_F(Gen4EngineTest, HighEvasionAndAccuracy) {
  // Reproduce branchProbability > FixType(0) assertion failure
  engine_->setAccuracy(16);
  auto team_a = TeamNonVolatile().addPokemon(
      PokemonNonVolatile()
          .setBase(pokedex_->pokemon("smeargle"))
          .addMove(pokedex_->move("mud-slap")));

  auto team_b = TeamNonVolatile().addPokemon(
      PokemonNonVolatile()
          .setBase(pokedex_->pokemon("smeargle"))
          .addMove(pokedex_->move("sweet scent")));

  auto environment_nv =
      std::make_shared<EnvironmentNonvolatile>(team_a, team_b, true);
  engine_->setEnvironment(environment_nv);

  PossibleEnvironments results = engine_->updateState(
      engine_->initialState(), Action::move(0), Action::move(0));
  auto currentState = results.where1Hit(TEAM_A);
  results.printStates();

  // Run 7 turns of accuracy/evasion debuffs
  for (int turn = 1; turn <= 7; ++turn) {
    std::cout << "Turn " << turn << ": Mud-slap vs Sweet Scent" << std::endl;
    // We want to ensure Mud-slap hits to keep the debuff loop going.
    // results.whereHit(TEAM_A) returns states where Team A's move hit.
    results =
        engine_->updateState(currentState, Action::move(0), Action::move(0));
    results.printStates();

    ASSERT_FALSE(results.empty())
        << "Engine returned no states on turn " << turn;

    // will throw if Mud-slap fails to hit
    currentState = results.where1Hit(TEAM_A);
  }

  std::cout << "Successfully completed 7 turns of Mud-slap vs Sweet Scent"
            << std::endl;
}


TEST_F(Gen4EngineTest, Swap) {
  auto team = TeamNonVolatile()
      .addPokemon(PokemonNonVolatile()
        .setBase(pokedex_->pokemon("torkoal"))
        .addMove(pokedex_->move("explosion")))
      .addPokemon(PokemonNonVolatile()
        .setBase(pokedex_->pokemon("squirtle"))
        .addMove(pokedex_->move("surf")));
  auto environment = EnvironmentNonvolatile(team, team, true);
  engine_->setEnvironment(environment);

  auto swap_squirtle = engine_->updateState(engine_->initialState(), Action::swap(1), Action::wait());
  auto torkoal_dead = engine_->updateState(
      swap_squirtle.where1(), Action::move(0), Action::wait());
  auto both_dead = engine_->updateState(
      swap_squirtle.where1(), Action::wait(), Action::move(0));

  // active pokemon has changed:
  EXPECT_EQ(engine_->initialState().getTeam(0).getICPKV(), 0);
  EXPECT_EQ(swap_squirtle.where1().getTeam(0).getICPKV(), 1);

  // pokemon may not swap to themselves:
  EXPECT_FALSE(engine_->isValidAction(engine_->initialState(), Action::swap(0), TEAM_A));
  EXPECT_FALSE(
      engine_->isValidAction(swap_squirtle.where1(), Action::swap(1), TEAM_A));
  // pokemon may swap when alive:
  EXPECT_TRUE(engine_->isValidAction(engine_->initialState(), Action::swap(1), TEAM_A));
  EXPECT_TRUE(engine_->isValidAction(engine_->initialState(), Action::swap(1), TEAM_B));
  // dead pokemon may swap:
  EXPECT_TRUE(
      engine_->isValidAction(torkoal_dead.where1(), Action::swap(1), TEAM_B));
  // living pokemon may NOT swap when the enemy is dead:
  EXPECT_FALSE(
      engine_->isValidAction(torkoal_dead.where1(), Action::swap(0), TEAM_A));
  EXPECT_TRUE(
      engine_->isValidAction(torkoal_dead.where1(), Action::wait(), TEAM_A));
  // if BOTH pokemon are dead, both pokemon may swap:
  EXPECT_TRUE(
      engine_->isValidAction(both_dead.where1(), Action::swap(0), TEAM_A));
  EXPECT_TRUE(
      engine_->isValidAction(both_dead.where1(), Action::swap(1), TEAM_B));
  // move counts should be accurate:
  EXPECT_EQ(
      engine_->getValidActions(torkoal_dead.where1().getEnv(), TEAM_B).size(),
      1);
}


TEST_F(Gen4EngineTest, InvalidAction) {
  engine_->setAllowInvalidMoves(false);

  auto team = TeamNonVolatile()
      .addPokemon(PokemonNonVolatile()
        .setBase(pokedex_->pokemon("charmander"))
        .addMove(pokedex_->move("cut")))
      .addPokemon(PokemonNonVolatile()
        .setBase(pokedex_->pokemon("bulbasaur"))
        .addMove(pokedex_->move("razor leaf")));
  auto environment = EnvironmentNonvolatile(team, team, true);
  engine_->setEnvironment(environment);

  auto constInitialState = engine_->initialState();

  // Create a mutable copy of the state
  auto mutableStateData = constInitialState.data();
  // Faint the second pokemon
  TeamStatus teamStatus{};
  PokemonVolatile faintedPokemon(
    constInitialState.nv().getTeam(TEAM_A).teammate(1),
    mutableStateData.teams[TEAM_A].teammates[1],
    teamStatus
  );
  faintedPokemon.setHP(0);

  // Create a new ConstEnvironmentVolatile for the test
  ConstEnvironmentVolatile initialState(constInitialState.nv(), mutableStateData);

  // Pokemon may not swap to a dead pokemon
  EXPECT_EQ(engine_->isValidAction(initialState, Action::swap(1), TEAM_A).reason, IsValidResult::SWITCH_POKEMON_DEAD);

  try {
    engine_->updateState(initialState, Action::swap(1), Action::wait());
    FAIL() << "Expected std::runtime_error";
  } catch(std::runtime_error const & err) {
    EXPECT_EQ(err.what(), std::string("Invalid Action for Team A: Cannot switch to a dead pokemon"));
  } catch(...) {
    FAIL() << "Expected std::runtime_error";
  }
}


