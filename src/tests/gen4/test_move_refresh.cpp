#include "engine_test.hpp"

class RefreshTest : public Gen4EngineTest {
 protected:
  void SetUp() override {
    Gen4EngineTest::SetUp();
  }

  void SetupTest(uint32_t ailment) {
      auto team_a = TeamNonVolatile()
          .addPokemon(PokemonNonVolatile()
              .setBase(pokedex_->pokemon("altaria"))
              .addMove(pokedex_->move("refresh"))
              .setLevel(100));

      auto team_b = TeamNonVolatile()
          .addPokemon(PokemonNonVolatile()
              .setBase(pokedex_->pokemon("squirtle"))
              .addMove(pokedex_->move("tackle"))
              .setLevel(100));

      // We don't necessarily need to use the member environment_nv,
      // but passing a temporary is fine as setEnvironment makes a copy.
      engine_->setEnvironment(EnvironmentNonvolatile(team_a, team_b, true));

      auto initialEnv = engine_->initialState();
      mutableData_ = initialEnv.data();

      // Set Ailment
      mutableData_.teams[0].teammates[0].status_nonvolatile = ailment;

      // If toxic, set tier
      if (ailment == AIL_NV_POISON_TOXIC) {
          mutableData_.teams[0].activeVolatiles[0].toxicPoison_tier = 1;
      }

      // Important: Use initialEnv.nv() so pointers match
      env_ = std::make_shared<EnvironmentVolatile>(initialEnv.nv(), mutableData_);
  }

  EnvironmentVolatileData mutableData_;
  std::shared_ptr<EnvironmentVolatile> env_;
};

TEST_F(RefreshTest, CuresBurn) {
  SetupTest(AIL_NV_BURN);
  auto result = engine_->updateState(*env_, Action::moveAlly(0, 0), Action::wait());
  ASSERT_GT(result.size(), 0);
  EXPECT_EQ(result.where1Hit(0).teammate(0, 0).getStatusAilment(), AIL_NV_NONE);
}

TEST_F(RefreshTest, CuresParalysis) {
  SetupTest(AIL_NV_PARALYSIS);
  auto result = engine_->updateState(*env_, Action::moveAlly(0, 0), Action::wait());
  ASSERT_GT(result.size(), 0);
  EXPECT_EQ(result.where1Hit(0).teammate(0, 0).getStatusAilment(), AIL_NV_NONE);
}

TEST_F(RefreshTest, CuresPoison) {
  SetupTest(AIL_NV_POISON);
  auto result = engine_->updateState(*env_, Action::moveAlly(0, 0), Action::wait());
  ASSERT_GT(result.size(), 0);
  EXPECT_EQ(result.where1Hit(0).teammate(0, 0).getStatusAilment(), AIL_NV_NONE);
}

TEST_F(RefreshTest, CuresToxicPoison) {
  SetupTest(AIL_NV_POISON_TOXIC);
  auto result = engine_->updateState(*env_, Action::moveAlly(0, 0), Action::wait());
  ASSERT_GT(result.size(), 0);
  auto pkv = result.where1Hit(0).teammate(0, 0);
  EXPECT_EQ(pkv.getStatusAilment(), AIL_NV_NONE);
  EXPECT_EQ(pkv.status().toxicPoison_tier, 0);
}

TEST_F(RefreshTest, FailsWhenHealthy) {
  SetupTest(AIL_NV_NONE);
  auto result = engine_->updateState(*env_, Action::move(0), Action::wait());
  ASSERT_GT(result.size(), 0);
  EXPECT_EQ(result.where1Hit(0).teammate(0, 0).getStatusAilment(), AIL_NV_NONE);
}
