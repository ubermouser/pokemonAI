#include <set>

#include "gen4/engine_test.hpp"
#include "mock_engine_test.hpp"
#include "pokemonai/roulette.h"


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
  EnvironmentBitfield target = EnvironmentBitfield().flagsFor(TEAM_A).setHit().setCrit();
  auto states = result.where(target);

  EXPECT_GT(states.size(), 0);
  for (const auto& state : states) {
    EXPECT_TRUE(state.flagsFor(TEAM_A).isHit());
    EXPECT_TRUE(state.flagsFor(TEAM_A).isCrit());
  }
}


TEST_F(EnvironmentPossibleSearchTest, WhereHit) {
  auto states = result.whereHit(0);
  EXPECT_GT(states.size(), 0);
  for (const auto& state : states) {
    EXPECT_TRUE(state.flagsFor(TEAM_A).isHit());
  }
}


TEST_F(EnvironmentPossibleSearchTest, WhereCrit) {
  auto states = result.whereCrit(0);
  EXPECT_GT(states.size(), 0);
  for (const auto& state : states) {
    EXPECT_TRUE(state.flagsFor(TEAM_A).isCrit());
  }
}


TEST_F(EnvironmentPossibleSearchTest, WhereStatus) {
  // Iron Head has 30% flinch (secondary effect)
  auto states = result.whereStatus(0);
  EXPECT_GT(states.size(), 0);
  for (const auto& state : states) {
    EXPECT_TRUE(state.flagsFor(TEAM_A).isSecondary());
  }
}


TEST_F(EnvironmentPossibleSearchTest, Where1Hit) {
  auto best = result.where1Hit(0);
  EXPECT_FALSE(best.isEmpty());
  EXPECT_TRUE(best.flagsFor(TEAM_A).isHit());
}


TEST_F(EnvironmentPossibleSearchTest, Where1Crit) {
  auto best = result.where1Crit(0);
  EXPECT_FALSE(best.isEmpty());
  EXPECT_TRUE(best.flagsFor(TEAM_A).isCrit());
}


TEST_F(EnvironmentPossibleSearchTest, Where1Status) {
  auto best = result.where1Status(0);
  EXPECT_FALSE(best.isEmpty());
  EXPECT_TRUE(best.flagsFor(TEAM_A).isSecondary());
}


TEST_F(EnvironmentPossibleSearchTest, Where1Probabilistic) {
  // Verify where1 actually finds the most probable state
  EnvironmentBitfield target = EnvironmentBitfield().flagsFor(TEAM_A).setHit();
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
  impossibleTarget.bits.team0[0].hit = 1;
  impossibleTarget.bits.team0[0].switched = 1; // Cannot both hit and switch in same unit of work
  EXPECT_THROW(result.where1(impossibleTarget), std::runtime_error);
}


TEST_F(EnvironmentPossibleSearchTest, WhereExcludesPruned) {
  // Find a state that is currently returned:
  EnvironmentBitfield target = EnvironmentBitfield().flagsFor(TEAM_A).setHit();
  auto initialCount = result.where(target).size();
  EXPECT_GT(initialCount, 0);

  // Manually mark one of the matching states as pruned:
  bool found = false;
  for (size_t i = 0; i < result.size(); ++i) {
    auto m = result.at(i).data().getBitmask();
    if ((m.collapseTeams() & target.collapseTeams()) == target.collapseTeams()) {
      const_cast<EnvironmentPossibleData&>(result.at(i).data()).getBitmask().bits.pruned = 1;
      found = true;
      break;
    } else {
       std::cerr << "PRUNE DEBUG: state " << i << " raw: " << std::hex << m.raw[0] << " collapsed: " << m.collapseTeams().raw[0] << " target_collapsed: " << target.collapseTeams().raw[0] << std::dec << std::endl;
    }
  }
  EXPECT_TRUE(found);

  // Verify the count has decreased:
  auto newCount = result.where(target).size();
  EXPECT_EQ(newCount, initialCount - 1);
}


TEST_F(EnvironmentPossibleSearchTest, SearchWithNegativeMask) {
  // Find states where team 0 hit but did NOT crit:
  EnvironmentBitfield mask = EnvironmentBitfield().flagsFor(TEAM_A).setHit().setCrit();
  EnvironmentBitfield expected =
      EnvironmentBitfield().flagsFor(TEAM_A).setHit();  // crit = 0

  auto states = result.where(mask, expected);

  EXPECT_GT(states.size(), 0);
  for (const auto& state : states) {
    EXPECT_TRUE(state.flagsFor(TEAM_A).isHit());
    EXPECT_FALSE(state.flagsFor(TEAM_A).isCrit());
  }
}


TEST_F(EnvironmentPossibleSearchTest, SearchWithPredicate) {
  // Find states where team 1's active pokemon has full HP (unrealistic in this
  // test context but tests the mechanic) Or more simply, find states where team
  // 0 moved first:
  auto states = result.where([](const ConstEnvironmentPossible& state) {
    return state.flagsFor(TEAM_A).isMovedFirst();
  });

  EXPECT_GT(states.size(), 0);
  for (const auto& state : states) { EXPECT_TRUE(state.flagsFor(TEAM_A).isMovedFirst()); }
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


class EnvironmentPossibleSelectTest : public MockEngineTest {
 protected:
  void SetUp() override {
    MockEngineTest::SetUp();
    spdlog::set_level(spdlog::level::warn);

    auto team = TeamNonVolatile().addPokemon(
        PokemonNonVolatile()
            .setBase(pokedex_->pokemon("test_pokemon"))
            .addMove(pokedex_->move("move_any_adjacent"))
            .setLevel(100));
    environment_nv = EnvironmentNonvolatile(team, team, true);
    engine_->setEnvironment(environment_nv);

    // Speed tie produces 18 states total: 9 unique states and 9 merged/pruned states
    result = engine_->updateState(
        engine_->initialState(), Action::move(0), Action::move(0));
  }

  PossibleEnvironments result;
};


TEST_F(EnvironmentPossibleSelectTest, StateSelectMostLikelyN_OrdersAndExcludesPruned) {
  ASSERT_EQ(result.size(), 18);
  ASSERT_EQ(result.getNumUnique(), 9);

  auto top4 = result.stateSelect_mostLikelyN(4);
  ASSERT_EQ(top4.size(), 4);
  EXPECT_EQ(top4[0], result.mostProbableIndex());

  std::set<size_t> uniqueIndices(top4.begin(), top4.end());
  EXPECT_EQ(uniqueIndices.size(), 4);

  for (size_t k = 0; k < top4.size(); ++k) {
    ASSERT_LT(top4[k], result.size());
    EXPECT_FALSE(result.at(top4[k]).isPruned());
    if (k + 1 < top4.size()) {
      FixType pCurr = result.at(top4[k]).getProbability();
      FixType pNext = result.at(top4[k + 1]).getProbability();
      EXPECT_TRUE(pCurr > pNext || (pCurr == pNext && top4[k] < top4[k + 1]));
    }
  }

  FixType cutoff = result.at(top4.back()).getProbability();
  for (size_t i = 0; i < result.size(); ++i) {
    if (result.at(i).isPruned() || uniqueIndices.count(i)) { continue; }
    EXPECT_GE(cutoff, result.at(i).getProbability());
  }
}


TEST_F(EnvironmentPossibleSelectTest, StateSelectMostLikelyN_EdgeCases) {
  EXPECT_TRUE(result.stateSelect_mostLikelyN(0).empty());

  auto all = result.stateSelect_mostLikelyN(100);
  EXPECT_EQ(all.size(), result.getNumUnique());
  std::set<size_t> uniqueIndices(all.begin(), all.end());
  EXPECT_EQ(uniqueIndices.size(), result.getNumUnique());
  for (size_t idx : all) {
    ASSERT_LT(idx, result.size());
    EXPECT_FALSE(result.at(idx).isPruned());
  }
}


TEST_F(EnvironmentPossibleSelectTest, StateSelectRouletteN_DistinctAndExcludesPruned) {
  srand(42);
  EXPECT_TRUE(result.stateSelect_rouletteN(0).empty());

  auto sampled = result.stateSelect_rouletteN(5);
  ASSERT_EQ(sampled.size(), 5);
  std::set<size_t> uniqueSampled(sampled.begin(), sampled.end());
  EXPECT_EQ(uniqueSampled.size(), 5);
  for (size_t idx : sampled) {
    ASSERT_LT(idx, result.size());
    EXPECT_FALSE(result.at(idx).isPruned());
  }

  auto capped = result.stateSelect_rouletteN(100);
  EXPECT_EQ(capped.size(), result.getNumUnique());
  std::set<size_t> uniqueCapped(capped.begin(), capped.end());
  EXPECT_EQ(uniqueCapped.size(), result.getNumUnique());
  for (size_t idx : capped) {
    ASSERT_LT(idx, result.size());
    EXPECT_FALSE(result.at(idx).isPruned());
  }
}


TEST_F(EnvironmentPossibleSelectTest, RouletteHandlesSubFloatEpsilonWeights) {
  srand(42);
  std::vector<double> tinyWeights = {0.0, 1e-12, 2e-12, 4e-12, 8e-12};

  size_t one = roulette<double>::select(tinyWeights);
  EXPECT_GE(one, 1);
  EXPECT_LT(one, tinyWeights.size());

  auto three = roulette<double>::selectDynamic(tinyWeights, 3);
  ASSERT_EQ(three.size(), 3);
  std::set<size_t> uniqueThree(three.begin(), three.end());
  EXPECT_EQ(uniqueThree.size(), 3);
  for (size_t idx : three) {
    EXPECT_GE(idx, 1);
    EXPECT_LT(idx, tinyWeights.size());
  }

  auto allValid = roulette<double>::selectDynamic(tinyWeights, 10);
  EXPECT_EQ(allValid.size(), 4);
}


TEST_F(EnvironmentPossibleSelectTest, SaneProbabilitiesHelper) {
  EXPECT_TRUE(result.saneProbabilities());

  PossibleEnvironments emptyEnv;
  EXPECT_FALSE(emptyEnv.saneProbabilities());

  PossibleEnvironments mutated = result;
  size_t nonPrunedIdx = mutated.mostProbableIndex();
  mutated.at(nonPrunedIdx).getProbability() = FixType(0);
  EXPECT_FALSE(mutated.saneProbabilities());
}
