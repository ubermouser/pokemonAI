#include <gtest/gtest.h>
#include <memory>
#include "pokemonai/pkai.h"
#include "pokemonai/game.h"
#include "pokemonai/gen4_scripts.h"
#include "pokemonai/pokedex.h"
#include "pokemonai/pokedex_static.h"
#include "pokemonai/pkCU.h"
#include "pokemonai/engine.h"
#include "pokemonai/pokemon_nonvolatile.h"

// Define a test fixture for Focus Sash tests
class FocusSashTest : public ::testing::Test {
protected:
    std::shared_ptr<PokedexStatic> pokedex_;
    std::shared_ptr<PkCU> engine_;

    void SetUp() override {
        // Initialize the Pokedex
        pokedex_ = std::make_shared<PokedexStatic>();

        // Initialize the engine
        engine_ = std::make_shared<PkCU>();
        // PkCU::initialize does not take arguments
        // But we need to ensure plugins are loaded. setEnvironment does that.
        // engine_->initialize(pokedex_.get()); // Removed arguments

        engine_->setAllowInvalidMoves(true);
    }

    void TearDown() override {
        engine_.reset();
        pokedex_.reset();
    }

    // Helper to get a Pokemon with specific stats and item
    PokemonNonVolatile createPokemon(const std::string& species, const std::string& item, int level = 100) {
        PokemonNonVolatile pnv;
        pnv.setBase(pokedex_->pokemon(species));
        pnv.setLevel(level);
        if (!item.empty()) {
            pnv.setInitialItem(pokedex_->item(item));
        }
        // Ensure it has at least one move
        pnv.addMove(pokedex_->move("tackle"));
        pnv.initialize();
        return pnv;
    }
};

TEST_F(FocusSashTest, FocusSashPreventsOHKO) {
    // Create two teams
    // Team A: Strong attacker
    // Team B: Weak defender with Focus Sash

    PokemonNonVolatile p1 = createPokemon("garchomp", "", 100);
    p1.addMove(pokedex_->move("earthquake"));

    PokemonNonVolatile p2 = createPokemon("magikarp", "focus sash", 1);

    TeamNonVolatile t1;
    t1.addPokemon(p1);

    TeamNonVolatile t2;
    t2.addPokemon(p2);

    EnvironmentNonvolatile env;
    env.teams[0] = t1;
    env.teams[1] = t2;

    // Set up the battle state
    engine_->setEnvironment(env);
    auto initialState = engine_->initialState();

    // Ensure p2 is at full HP
    ConstPokemonVolatile pkv2 = initialState.getTeam(1).teammate(0);
    ASSERT_EQ(pkv2.getPercentHP(), 1.0);
    ASSERT_TRUE(pkv2.hasItem());
    ASSERT_EQ(pkv2.getItem().getName(), "focus sash");

    // Simulate turn: p1 attacks p2 with Earthquake (should be OHKO)
    // p1 uses Earthquake (index 1, as index 0 is Tackle)
    // p2 uses Tackle (index 0)

    auto result = engine_->updateState(initialState, Action::move(1), Action::move(0));

    bool foundSurvival = false;
    for (size_t i = 0; i < result.size(); ++i) {
        auto state = result.at(i);
        // We need to look at the environment in the state
        ConstPokemonVolatile victim = state.getEnv().getTeam(1).teammate(0);

        // We expect the victim to survive with 1 HP if Focus Sash works
        if (victim.isAlive()) {
            if (victim.getHP() == 1) {
                 // Check if item is consumed
                 if (!victim.hasItem()) {
                    foundSurvival = true;
                    break;
                 }
            }
        }
    }

    // For reproduction, we assert that we found the survival case.
    ASSERT_TRUE(foundSurvival) << "Focus Sash did not prevent OHKO";
}

TEST_F(FocusSashTest, FocusSashConsumedAfterUse) {
    // Similar to above, but check item consumption
     PokemonNonVolatile p1 = createPokemon("garchomp", "", 100);
    p1.addMove(pokedex_->move("earthquake"));

    PokemonNonVolatile p2 = createPokemon("magikarp", "focus sash", 1);

    TeamNonVolatile t1; t1.addPokemon(p1);
    TeamNonVolatile t2; t2.addPokemon(p2);
    EnvironmentNonvolatile env; env.teams[0] = t1; env.teams[1] = t2;

    engine_->setEnvironment(env);

    auto result = engine_->updateState(engine_->initialState(), Action::move(1), Action::move(0));

    bool foundSurvival = false;
    for (size_t i = 0; i < result.size(); ++i) {
        auto state = result.at(i);
        ConstPokemonVolatile victim = state.getEnv().getTeam(1).teammate(0);

        if (victim.isAlive() && victim.getHP() == 1) {
             foundSurvival = true;
            ASSERT_FALSE(victim.hasItem()) << "Focus Sash should be consumed";
        }
    }
    ASSERT_TRUE(foundSurvival);
}

TEST_F(FocusSashTest, FocusSashDoesNotWorkIfNotFullHP) {
     PokemonNonVolatile p1 = createPokemon("garchomp", "", 100);
    p1.addMove(pokedex_->move("earthquake"));

    PokemonNonVolatile p2 = createPokemon("magikarp", "focus sash", 1);

    TeamNonVolatile t1; t1.addPokemon(p1);
    TeamNonVolatile t2; t2.addPokemon(p2);
    EnvironmentNonvolatile env; env.teams[0] = t1; env.teams[1] = t2;

    engine_->setEnvironment(env);
    auto constInitialState = engine_->initialState();

    // Modify HP
    auto mutableStateData = constInitialState.data();

    // Getting Max HP from non-volatile
    // Use getFV_base(FV_HITPOINTS)
    uint32_t maxHP = constInitialState.nv().getTeam(1).teammate(0).getFV_base(FV_HITPOINTS);
    mutableStateData.teams[1].teammates[0].HPcurrent = maxHP - 1;

    // Create new state
    ConstEnvironmentVolatile modifiedState(constInitialState.nv(), mutableStateData);

    ConstPokemonVolatile victimStart = modifiedState.getTeam(1).teammate(0);
    ASSERT_NE(victimStart.getHP(), maxHP);
    ASSERT_EQ(victimStart.getHP(), maxHP - 1);

    auto result = engine_->updateState(modifiedState, Action::move(1), Action::move(0));

     for (size_t i = 0; i < result.size(); ++i) {
        auto state = result.at(i);
        ConstPokemonVolatile victim = state.getEnv().getTeam(1).teammate(0);

        // Should be dead
        ASSERT_FALSE(victim.isAlive()) << "Focus Sash should not work if HP is not full";
    }
}
