#include "engine_test.hpp"


class EnvironmentPossibleSearchTest : public Gen4EngineTest {
 protected:
  void SetUp() override {
    Gen4EngineTest::SetUp();

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

    result = engine_->updateState(
        engine_->initialState(), Action::move(0), Action::move(0));
  }

  PossibleEnvironments result;
};


TEST_F(EnvironmentPossibleSearchTest, SearchWithWhere) {
  // Find states where BOTH hit and crit occurred for team 0:
  EnvironmentBitfield target = EnvironmentBitfield().team(0).hasHit().hasCrit();
  auto states = result.where(target);

  EXPECT_GT(states.size(), 0);
  for (const auto& state : states) {
    EXPECT_TRUE(state.hasHit(0));
    EXPECT_TRUE(state.hasCrit(0));
  }
}


TEST_F(EnvironmentPossibleSearchTest, WhereHit) {
  auto states = result.whereHit(0);
  EXPECT_GT(states.size(), 0);
  for (const auto& state : states) {
    EXPECT_TRUE(state.hasHit(0));
  }
}


TEST_F(EnvironmentPossibleSearchTest, WhereCrit) {
  auto states = result.whereCrit(0);
  EXPECT_GT(states.size(), 0);
  for (const auto& state : states) {
    EXPECT_TRUE(state.hasCrit(0));
  }
}


TEST_F(EnvironmentPossibleSearchTest, WhereStatus) {
  // Iron Head has 30% flinch (secondary effect)
  auto states = result.whereStatus(0);
  EXPECT_GT(states.size(), 0);
  for (const auto& state : states) {
    EXPECT_TRUE(state.hasSecondary(0));
  }
}


TEST_F(EnvironmentPossibleSearchTest, Where1Hit) {
  auto best = result.where1Hit(0);
  EXPECT_FALSE(best.isEmpty());
  EXPECT_TRUE(best.hasHit(0));
}


TEST_F(EnvironmentPossibleSearchTest, Where1Crit) {
  auto best = result.where1Crit(0);
  EXPECT_FALSE(best.isEmpty());
  EXPECT_TRUE(best.hasCrit(0));
}


TEST_F(EnvironmentPossibleSearchTest, Where1Status) {
  auto best = result.where1Status(0);
  EXPECT_FALSE(best.isEmpty());
  EXPECT_TRUE(best.hasSecondary(0));
}


TEST_F(EnvironmentPossibleSearchTest, Where1Probabilistic) {
  // Verify where1 actually finds the most probable state
  EnvironmentBitfield target = EnvironmentBitfield().team(0).hasHit();
  auto allHits = result.where(target);
  auto expectedBest = *std::max_element(allHits.begin(), allHits.end(), [](const auto& a, const auto& b){
    return a.getProbability() < b.getProbability();
  });
  
  auto best = result.where1(target);
  EXPECT_EQ(best.getProbability(), expectedBest.getProbability());
}


TEST_F(EnvironmentPossibleSearchTest, Where1Throws) {
  // Verify where1 throws an exception if no state matches:
  EnvironmentBitfield impossibleTarget{};
  impossibleTarget.bits.team0.hit = 1;
  impossibleTarget.bits.team0.switched = 1; // Cannot both hit and switch in same unit of work
  EXPECT_THROW(result.where1(impossibleTarget), std::runtime_error);
}


TEST_F(EnvironmentPossibleSearchTest, WhereExcludesPruned) {
  // Find a state that is currently returned:
  EnvironmentBitfield target = EnvironmentBitfield().team(0).hasHit();
  auto initialCount = result.where(target).size();
  EXPECT_GT(initialCount, 0);

  // Manually mark one of the matching states as pruned:
  bool found = false;
  for (size_t i = 0; i < result.size(); ++i) {
    if ((result.at(i).data().flags.raw & target.raw) == target.raw) {
      const_cast<EnvironmentPossibleData&>(result.at(i).data()).flags.bits.pruned = 1;
      found = true;
      break;
    }
  }
  EXPECT_TRUE(found);

  // Verify the count has decreased:
  auto newCount = result.where(target).size();
  EXPECT_EQ(newCount, initialCount - 1);
}


TEST_F(EnvironmentPossibleSearchTest, SearchWithNegativeMask) {
  // Find states where team 0 hit but did NOT crit:
  EnvironmentBitfield mask = EnvironmentBitfield().team(0).hasHit().hasCrit();
  EnvironmentBitfield expected =
      EnvironmentBitfield().team(0).hasHit();  // crit = 0

  auto states = result.where(mask, expected);

  EXPECT_GT(states.size(), 0);
  for (const auto& state : states) {
    EXPECT_TRUE(state.hasHit(0));
    EXPECT_FALSE(state.hasCrit(0));
  }
}


TEST_F(EnvironmentPossibleSearchTest, SearchWithPredicate) {
  // Find states where team 1's active pokemon has full HP (unrealistic in this
  // test context but tests the mechanic) Or more simply, find states where team
  // 0 moved first:
  auto states = result.where([](const ConstEnvironmentPossible& state) {
    return state.hasMovedFirst(0);
  });

  EXPECT_GT(states.size(), 0);
  for (const auto& state : states) { EXPECT_TRUE(state.hasMovedFirst(0)); }
}


TEST_F(EnvironmentPossibleSearchTest, Where1NoArg) {
  // Find the single most probable state:
  auto expectedBest = *std::max_element(
      result.begin(), result.end(), [](const auto& a, const auto& b) {
        if (a.isPruned()) return true;
        if (b.isPruned()) return false;
        return a.probability < b.probability;
      });

  auto best = result.where1();
  EXPECT_EQ(best.getProbability(), expectedBest.probability);
}
