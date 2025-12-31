#include "engine_test.hpp"

TEST_F(EngineTest, PrimaryHitAndCrit) {
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
  EXPECT_EQ(result.at(0).hasHit(0), true);
  EXPECT_EQ(result.at(1).hasHit(0), false);
  EXPECT_EQ(result.at(2).hasCrit(0), true);
}


TEST_F(EngineTest, HighEngineAccuracy) {
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
  result.printStates();
}


TEST_F(EngineTest, Swap) {
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
  auto torkoal_dead = engine_->updateState(swap_squirtle.at(0), Action::move(0), Action::wait());
  auto both_dead = engine_->updateState(swap_squirtle.at(0), Action::wait(), Action::move(0));

  // active pokemon has changed:
  EXPECT_EQ(engine_->initialState().getTeam(0).getICPKV(), 0);
  EXPECT_EQ(swap_squirtle.at(0).getEnv().getTeam(0).getICPKV(), 1);

  // pokemon may not swap to themselves:
  EXPECT_FALSE(engine_->isValidAction(engine_->initialState(), Action::swap(0), TEAM_A));
  EXPECT_FALSE(engine_->isValidAction(swap_squirtle.at(0), Action::swap(1), TEAM_A));
  // pokemon may swap when alive:
  EXPECT_TRUE(engine_->isValidAction(engine_->initialState(), Action::swap(1), TEAM_A));
  EXPECT_TRUE(engine_->isValidAction(engine_->initialState(), Action::swap(1), TEAM_B));
  // dead pokemon may swap:
  EXPECT_TRUE(engine_->isValidAction(torkoal_dead.at(0), Action::swap(1), TEAM_B));
  // living pokemon may NOT swap when the enemy is dead:
  EXPECT_FALSE(engine_->isValidAction(torkoal_dead.at(0), Action::swap(0), TEAM_A));
  EXPECT_TRUE(engine_->isValidAction(torkoal_dead.at(0), Action::wait(), TEAM_A));
  // if BOTH pokemon are dead, both pokemon may swap:
  EXPECT_TRUE(engine_->isValidAction(both_dead.at(0), Action::swap(0), TEAM_A));
  EXPECT_TRUE(engine_->isValidAction(both_dead.at(0), Action::swap(1), TEAM_B));
  // move counts should be accurate:
  EXPECT_EQ(engine_->getValidActions(torkoal_dead.at(0).getEnv(), TEAM_B).size(), 1);
}


TEST_F(EngineTest, InvalidAction) {
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


TEST_F(EngineTest, GroundConditions) {
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
    auto spikes_removed = engine_->updateState(spikes.at(0), Action::wait(), Action::move(3));
    auto removed_vs_spikes = engine_->updateState(spikes_removed.at(0), Action::wait(), Action::swap(1));
    EXPECT_EQ(removed_vs_spikes.at(0).getEnv().getTeam(1).getPKV().getPercentHP(), 1.); // 100%
  }
  { // test normal harmed vs spikes:
    auto normal_vs_spikes = engine_->updateState(spikes.at(0), Action::wait(), Action::swap(1));
    EXPECT_NEAR(normal_vs_spikes.at(0).getEnv().getTeam(1).getPKV().getPercentHP(), 0.875, 0.005); // 87.5%
  }
  { // test normal harmed vs toxic spikes:
    auto normal_vs_toxic = engine_->updateState(toxic_spikes.at(0), Action::wait(), Action::swap(1));
    EXPECT_NEAR(normal_vs_toxic.at(0).getEnv().getTeam(1).getPKV().getPercentHP(), 0.875, 0.005); // 87.5%
    EXPECT_EQ(normal_vs_toxic.at(0).getEnv().getTeam(1).getPKV().getStatusAilment(), AIL_NV_POISON); // 87.5%
  }
  { // test levitate unharmed vs spikes:
    auto lev_vs_spikes = engine_->updateState(spikes.at(0), Action::wait(), Action::swap(2));
    EXPECT_EQ(lev_vs_spikes.at(0).getEnv().getTeam(1).getPKV().getPercentHP(), 1.); // 100%
  }
  { // test levitate unharmed vs toxic spikes:
    auto lev_vs_toxic = engine_->updateState(toxic_spikes.at(0), Action::wait(), Action::swap(2));
    EXPECT_EQ(lev_vs_toxic.at(0).getEnv().getTeam(1).getPKV().getPercentHP(), 1.); // 100%
  }
  { // test levitate harmed vs stealth rock:
    auto lev_vs_sr = engine_->updateState(stealth_rock.at(0), Action::wait(), Action::swap(2));
    EXPECT_NEAR(lev_vs_sr.at(0).getEnv().getTeam(1).getPKV().getPercentHP(), 0.875, 0.005); // 87.5%
  }
  { // test flying unharmed vs spikes:
    auto flying_vs_spikes = engine_->updateState(spikes.at(0), Action::wait(), Action::swap(3));
    EXPECT_EQ(flying_vs_spikes.at(0).getEnv().getTeam(1).getPKV().getPercentHP(), 1.); // 100%
  }
  { // test flying harmed vs stealth rock:
    auto flying_vs_sr = engine_->updateState(stealth_rock.at(0), Action::wait(), Action::swap(3));
    EXPECT_NEAR(flying_vs_sr.at(0).getEnv().getTeam(1).getPKV().getPercentHP(), 0.75, 0.005); // 75%
  }
}


TEST_F(EngineTest, LifeOrb) {
  auto team_1 = TeamNonVolatile()
      .addPokemon(PokemonNonVolatile()
        .setBase(pokedex_->pokemon("dusknoir"))
        .addMove(pokedex_->move("pain split"))
        .addMove(pokedex_->move("shadow punch"))
        .addMove(pokedex_->move("will-o-wisp"))
        .addMove(pokedex_->move("calm mind"))
        .setInitialItem(pokedex_->item("life orb"))
        .setLevel(100));
  auto team_2 = team_1;
  team_2.teammate(0).setNoInitialItem();
  auto environment = EnvironmentNonvolatile(team_1, team_2, true);
  engine_->setEnvironment(environment);

  auto sp_lifeorb = engine_->updateState(engine_->initialState(), Action::move(1), Action::wait());
  auto sp_noitem = engine_->updateState(engine_->initialState(), Action::wait(), Action::move(1));
  auto will_o_wisp = engine_->updateState(engine_->initialState(), Action::move(2), Action::wait());
  auto split_pain = engine_->updateState(sp_lifeorb.at(0), Action::move(0), Action::wait());
  auto calm_mind = engine_->updateState(engine_->initialState(), Action::move(3), Action::wait());

  { // attacking move: life orb subtracts 10%, damage is increased by 30%
    EXPECT_EQ(sp_lifeorb.at(0).getEnv().getTeam(0).teammate(0).getHP(), 180); // 90%
    EXPECT_EQ(sp_lifeorb.at(0).getEnv().getTeam(1).teammate(0).getHP(), 62); // 31%
    EXPECT_LT(sp_lifeorb.at(0).getEnv().getTeam(1).teammate(0).getHP(),
              sp_noitem.at(0).getEnv().getTeam(0).teammate(0).getHP());
  }
  { // special move targeting other team: no effect
    EXPECT_EQ(split_pain.at(0).getEnv().getTeam(0).teammate(0).getHP(),
              split_pain.at(0).getEnv().getTeam(1).teammate(0).getHP());
  }
  { // status move targeting other team: no effect
    EXPECT_EQ(will_o_wisp.at(0).getEnv().getTeam(0).teammate(0).getPercentHP(), 1.);
  }
  { // move targeting friendly team: no effect
    EXPECT_EQ(calm_mind.at(0).getEnv().getTeam(0).teammate(0).getPercentHP(), 1.);
  }
}

TEST_F(EngineTest, Flinch) {
  auto team_a = TeamNonVolatile().addPokemon(
      PokemonNonVolatile()
          .setBase(pokedex_->pokemon("jirachi"))
          .addMove(pokedex_->move("iron head"))
          .setLevel(100));

  auto team_b = TeamNonVolatile().addPokemon(
      PokemonNonVolatile()
          .setBase(pokedex_->pokemon("charmander"))
          .addMove(pokedex_->move("ember"))
          .setLevel(100));

  auto environment = EnvironmentNonvolatile(team_a, team_b, true);
  engine_->setEnvironment(environment);

  // Turn 1: Jirachi uses Iron Head, Mew uses Tackle
  // Iron Head has 30% chance to flinch.
  auto result = engine_->updateState(
      engine_->initialState(), Action::move(0), Action::move(0));
  result.printStates();

  // Jirachi's health should be full if Charmander flinches
  EXPECT_EQ(result.at(0).getEnv().getTeam(0).teammate(0).getMissingHP(), 0);
  EXPECT_GE(result.at(0).getEnv().getTeam(1).teammate(0).getMissingHP(), 100);
}
