#include "engine_test.hpp"

class FocusSashTest : public EngineTest {
protected:
    void SetUp() override {
        EngineTest::SetUp();

        // Team A:
        // 1. Garchomp (Strong attacker)
        // 2. Bulbasaur (Weak attacker for chip damage)
        auto team_a = TeamNonVolatile()
            .addPokemon(PokemonNonVolatile()
                .setBase(pokedex_->pokemon("garchomp"))
                .addMove(pokedex_->move("earthquake"))
                .setLevel(100))
            .addPokemon(PokemonNonVolatile()
                .setBase(pokedex_->pokemon("bulbasaur"))
                .addMove(pokedex_->move("tackle"))
                .setLevel(5));

        // Team B: Weak defender (Magikarp) with Focus Sash
        // Increased level slightly to 5 to ensure it survives a Level 5 Tackle but dies to Lvl 100 Earthquake
        auto team_b = TeamNonVolatile()
            .addPokemon(PokemonNonVolatile()
                .setBase(pokedex_->pokemon("magikarp"))
                .setInitialItem(pokedex_->item("focus sash"))
                .addMove(pokedex_->move("splash"))
                .setLevel(5));

        env_ptr = std::make_shared<EnvironmentNonvolatile>(team_a, team_b, true);
        engine_->setEnvironment(env_ptr);

        setup_full_hp = engine_->initialState();

        // Setup state where Magikarp is damaged
        // 1. Switch to Bulbasaur
        auto swap_state = engine_->updateState(setup_full_hp, Action::swap(1), Action::move(0));

        // 2. Bulbasaur uses Tackle. Magikarp uses Splash.
        // We need to find the state where Bulbasaur moved and hit.
        auto attack_result = engine_->updateState(swap_state.at(0), Action::move(0), Action::move(0));

        // Find a resulting environment where Magikarp is damaged but alive
        for(size_t i = 0; i < attack_result.size(); ++i) {
            auto env = attack_result.at(i).getEnv();
            auto victim = env.getTeam(1).getPKV();
            if (victim.isAlive() && victim.getHP() < victim.nv().getMaxHP()) {
                // 3. Switch back to Garchomp
                auto return_swap = engine_->updateState(attack_result.at(i), Action::swap(0), Action::move(0));
                setup_damaged_data = return_swap.at(0).getEnv().data();
                return;
            }
        }
    }

    ConstEnvironmentVolatile setup_full_hp = ConstEnvironmentVolatile(EnvironmentNonvolatile(), EnvironmentVolatileData());
    EnvironmentVolatileData setup_damaged_data;
    std::shared_ptr<EnvironmentNonvolatile> env_ptr;
};

TEST_F(FocusSashTest, FocusSashPreventsOHKO) {
    // Garchomp uses Earthquake (index 0) on full HP Magikarp
    auto result = engine_->updateState(setup_full_hp, Action::move(0), Action::move(0));

    bool foundSurvival = false;
    for (size_t i = 0; i < result.size(); ++i) {
        auto env = result.at(i).getEnv();
        ConstPokemonVolatile victim = env.getTeam(1).getPKV();

        // Should survive with 1 HP
        if (victim.isAlive() && victim.getHP() == 1) {
            foundSurvival = true;
            break;
        }
    }
    ASSERT_TRUE(foundSurvival) << "Focus Sash did not prevent OHKO";
}

TEST_F(FocusSashTest, FocusSashConsumedAfterUse) {
    // Garchomp uses Earthquake (index 0) on full HP Magikarp
    auto result = engine_->updateState(setup_full_hp, Action::move(0), Action::move(0));

    bool foundSurvival = false;
    for (size_t i = 0; i < result.size(); ++i) {
        auto env = result.at(i).getEnv();
        ConstPokemonVolatile victim = env.getTeam(1).getPKV();

        if (victim.isAlive() && victim.getHP() == 1) {
            foundSurvival = true;
            EXPECT_FALSE(victim.hasItem()) << "Focus Sash should be consumed";
        }
    }
    ASSERT_TRUE(foundSurvival);
}

TEST_F(FocusSashTest, FocusSashDoesNotWorkIfNotFullHP) {
    // Verify setup_damaged state
    ConstEnvironmentVolatile env(*env_ptr, setup_damaged_data);

    ConstPokemonVolatile victimStart = env.getTeam(1).getPKV();
    if (victimStart.getHP() == 0) {
         FAIL() << "Failed to setup damaged environment in SetUp()";
    }

    ASSERT_LT(victimStart.getHP(), victimStart.nv().getMaxHP());
    ASSERT_TRUE(victimStart.isAlive());
    ASSERT_TRUE(victimStart.hasItem());

    // Garchomp uses Earthquake (index 0) on damaged Magikarp
    auto result = engine_->updateState(env, Action::move(0), Action::move(0));

    for (size_t i = 0; i < result.size(); ++i) {
        auto result_env = result.at(i).getEnv();
        ConstPokemonVolatile victim = result_env.getTeam(1).getPKV();

        // Should be dead
        EXPECT_FALSE(victim.isAlive()) << "Focus Sash should not activate if HP is not full";
    }
}
