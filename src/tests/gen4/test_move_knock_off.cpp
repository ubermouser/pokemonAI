#include "engine_test.hpp"

class KnockOffTest : public Gen4EngineTest {
 protected:
  void SetUp() override {
    Gen4EngineTest::SetUp();

    auto team_a = TeamNonVolatile()
        .addPokemon(PokemonNonVolatile()
          .setBase(pokedex_->pokemon("alakazam"))
          .addMove(pokedex_->move("knock off"))
          .setLevel(100));
    auto team_b = TeamNonVolatile()
        .addPokemon(PokemonNonVolatile()
          .setBase(pokedex_->pokemon("blissey"))
          .setInitialItem(pokedex_->item("leftovers"))
          .setLevel(100))
        .addPokemon(PokemonNonVolatile()
          .setBase(pokedex_->pokemon("muk"))
          .setAbility(pokedex_->ability("sticky hold"))
          .setInitialItem(pokedex_->item("leftovers"))
          .setLevel(100))
        .addPokemon(PokemonNonVolatile()
          .setBase(pokedex_->pokemon("gengar"))
          .addMove(pokedex_->move("substitute"))
          .setInitialItem(pokedex_->item("life orb"))
          .setLevel(100));

    environment_nv = EnvironmentNonvolatile(team_a, team_b, true);
    engine_->setEnvironment(environment_nv);
  }
};

TEST_F(KnockOffTest, removes_item) {
  // Turn 1: Alakazam uses Knock Off vs Blissey (holding Leftovers)
  auto turn1 = engine_->updateState(
    engine_->initialState(), Action::move(0), Action::wait());

  auto final_env_v = turn1.where1().getEnv();
  // Expect Blissey to have lost Leftovers
  EXPECT_FALSE(final_env_v.getTeam(1).getPKV().hasItem());
}

TEST_F(KnockOffTest, sticky_hold) {
  // Swap to Muk (Sticky Hold) first
  auto swap_muk = engine_->updateState(
    engine_->initialState(), Action::wait(), Action::swap(1));

  auto turn1 = engine_->updateState(
    swap_muk.where1(), Action::move(0), Action::wait());

  auto final_env_v = turn1.where1().getEnv();
  // Expect Muk to still have Leftovers
  EXPECT_TRUE(final_env_v.getTeam(1).getPKV().hasItem());
  EXPECT_EQ(final_env_v.getTeam(1).getPKV().getItem().getName(), "leftovers");
}

TEST_F(KnockOffTest, substitute_active) {
  // Swap to Gengar
  auto swap_gengar = engine_->updateState(
    engine_->initialState(), Action::wait(), Action::swap(2));

  // Gengar uses Substitute
  auto substitute_up = engine_->updateState(
    swap_gengar.where1(), Action::wait(), Action::move(0));

  // Verify substitute is up
  EXPECT_GT(substitute_up.where1().getEnv().getTeam(1).getPKV().status().cTeammate.substitute, 0);

  // Alakazam uses Knock Off
  auto knock_off = engine_->updateState(
      substitute_up.where1(), Action::move(0), Action::wait());

  auto final_env_v = knock_off.where1().getEnv();

  // Expect Gengar to still have Life Orb
  EXPECT_TRUE(final_env_v.getTeam(1).getPKV().hasItem());
  EXPECT_EQ(final_env_v.getTeam(1).getPKV().getItem().getName(), "life orb");
}

TEST_F(KnockOffTest, no_item) {
  // Blissey starts with item. Remove it first.

  // Turn 1: Remove item
  auto turn1 = engine_->updateState(
    engine_->initialState(), Action::move(0), Action::wait());

  // Turn 2: Knock Off again
  auto turn2 = engine_->updateState(
      turn1.where1(), Action::move(0), Action::wait());

  auto final_env_v = turn2.where1().getEnv();
  EXPECT_FALSE(final_env_v.getTeam(1).getPKV().hasItem());
}

TEST_F(KnockOffTest, DISABLED_arceus_plate) {
  // Setup Arceus with Draco Plate
  auto team_c = TeamNonVolatile()
    .addPokemon(PokemonNonVolatile()
      .setBase(pokedex_->pokemon("arceus"))
      .setInitialItem(pokedex_->item("draco plate"))
      .setLevel(100));

  // Replace team B
  environment_nv = EnvironmentNonvolatile(environment_nv.getTeam(0), team_c, true);
  engine_->setEnvironment(environment_nv);

  // Alakazam uses Knock Off
  auto turn1 = engine_->updateState(
    engine_->initialState(), Action::move(0), Action::wait());

  auto final_env_v = turn1.where1().getEnv();
  // Expect Arceus to still have Draco Plate
  EXPECT_TRUE(final_env_v.getTeam(1).getPKV().hasItem());
  EXPECT_EQ(final_env_v.getTeam(1).getPKV().getItem().getName(), "draco plate");
}
