#include <gtest/gtest.h>
#include <spdlog/spdlog.h>

#include <array>
#include <bitset>
#include <memory>
#include <vector>

#include "pokemonai/engine.h"
#include "pokemonai/pkCU.h"
#include "pokemonai/pokedex_static.h"
#include "pokemonai/pokemon_nonvolatile.h"
#include "pokemonai/team_nonvolatile.h"

// Global pointer needed by some classes
extern PKAISHARED const Pokedex* pkdex;
static std::array<int, PLUGIN_MAXSIZE> plugin_calls;


// Mock plugin functions with correct signatures from pluggable_types.h
int mock_onInitMove(PokemonNonVolatile&, MoveNonVolatile&) {
  SPDLOG_TRACE("PLUGIN_ON_INIT: mock_onInitMove");
  plugin_calls[PLUGIN_ON_INIT]++;
  return 0;
}

int mock_onReset(PkCUEngine&, void*) {
  SPDLOG_TRACE("PLUGIN_ON_RESET: mock_onReset");
  plugin_calls[PLUGIN_ON_RESET]++;
  return 0;
}

int mock_onSetSpeedBracket(
    PkCUEngine&, MoveVolatile, PokemonVolatile, int32_t&) {
  SPDLOG_TRACE("PLUGIN_ON_SETSPEEDBRACKET: mock_onSetSpeedBracket");
  plugin_calls[PLUGIN_ON_SETSPEEDBRACKET]++;
  return 0;
}

int mock_onModifySpeed(PkCUEngine&, PokemonVolatile, uint32_t&) {
  SPDLOG_TRACE("PLUGIN_ON_MODIFYSPEED: mock_onModifySpeed");
  plugin_calls[PLUGIN_ON_MODIFYSPEED]++;
  return 0;
}

int mock_onBeginningOfTurn(PkCUEngine&, PokemonVolatile) {
  SPDLOG_TRACE("PLUGIN_ON_BEGINNINGOFTURN: mock_onBeginningOfTurn");
  plugin_calls[PLUGIN_ON_BEGINNINGOFTURN]++;
  return 0;
}

int mock_onEvaluateMove(
    PkCUEngine&, MoveVolatile, PokemonVolatile, PokemonVolatile) {
  SPDLOG_TRACE("PLUGIN_ON_EVALUATEMOVE: mock_onEvaluateMove");
  plugin_calls[PLUGIN_ON_EVALUATEMOVE]++;
  return 0;
}

int mock_onSetBasePower(
    PkCUEngine&, MoveVolatile, PokemonVolatile, PokemonVolatile, uint32_t&) {
  SPDLOG_TRACE("PLUGIN_ON_SETBASEPOWER: mock_onSetBasePower");
  plugin_calls[PLUGIN_ON_SETBASEPOWER]++;
  return 0;
}

int mock_onModifyBasePower(
    PkCUEngine&, MoveVolatile, PokemonVolatile, PokemonVolatile, fpType&) {
  SPDLOG_TRACE("PLUGIN_ON_MODIFYBASEPOWER: mock_onModifyBasePower");
  plugin_calls[PLUGIN_ON_MODIFYBASEPOWER]++;
  return 0;
}

int mock_onModifyAttackPower(
    PkCUEngine&, MoveVolatile, PokemonVolatile, PokemonVolatile, fpType&) {
  SPDLOG_TRACE("PLUGIN_ON_MODIFYATTACKPOWER: mock_onModifyAttackPower");
  plugin_calls[PLUGIN_ON_MODIFYATTACKPOWER]++;
  return 0;
}

int mock_onModifyCriticalPower(
    PkCUEngine&, MoveVolatile, PokemonVolatile, PokemonVolatile, fpType&) {
  SPDLOG_TRACE("PLUGIN_ON_MODIFYCRITICALPOWER: mock_onModifyCriticalPower");
  plugin_calls[PLUGIN_ON_MODIFYCRITICALPOWER]++;
  return 0;
}

int mock_onModifyRawDamage(
    PkCUEngine&, MoveVolatile, PokemonVolatile, PokemonVolatile, fpType&) {
  SPDLOG_TRACE("PLUGIN_ON_MODIFYRAWDAMAGE: mock_onModifyRawDamage");
  plugin_calls[PLUGIN_ON_MODIFYRAWDAMAGE]++;
  return 0;
}

int mock_onSetMoveType(
    PkCUEngine&, MoveVolatile, PokemonVolatile, PokemonVolatile, const Type*&) {
  SPDLOG_TRACE("PLUGIN_ON_SETMOVETYPE: mock_onSetMoveType");
  plugin_calls[PLUGIN_ON_SETMOVETYPE]++;
  return 0;
}

int mock_onModifySTAB(
    PkCUEngine&, MoveVolatile, PokemonVolatile, PokemonVolatile, fpType&) {
  SPDLOG_TRACE("PLUGIN_ON_MODIFYSTAB: mock_onModifySTAB");
  plugin_calls[PLUGIN_ON_MODIFYSTAB]++;
  return 0;
}

int mock_onSetDefenseType(
    PkCUEngine&,
    const Type&,
    MoveVolatile,
    PokemonVolatile,
    PokemonVolatile,
    fpType&) {
  SPDLOG_TRACE("PLUGIN_ON_SETDEFENSETYPE: mock_onSetDefenseType");
  plugin_calls[PLUGIN_ON_SETDEFENSETYPE]++;
  return 0;
}

int mock_onModifyItemPower(
    PkCUEngine&, MoveVolatile, PokemonVolatile, PokemonVolatile, fpType&) {
  SPDLOG_TRACE("PLUGIN_ON_MODIFYITEMPOWER: mock_onModifyItemPower");
  plugin_calls[PLUGIN_ON_MODIFYITEMPOWER]++;
  return 0;
}

int mock_onModifyHitProbability(
    PkCUEngine&, MoveVolatile, PokemonVolatile, PokemonVolatile, FixType&) {
  SPDLOG_TRACE("PLUGIN_ON_MODIFYHITPROBABILITY: mock_onModifyHitProbability");
  plugin_calls[PLUGIN_ON_MODIFYHITPROBABILITY]++;
  return 0;
}

int mock_onModifyCritProbability(
    PkCUEngine&, MoveVolatile, PokemonVolatile, PokemonVolatile, FixType&) {
  SPDLOG_TRACE("PLUGIN_ON_MODIFYCRITPROBABILITY: mock_onModifyCritProbability");
  plugin_calls[PLUGIN_ON_MODIFYCRITPROBABILITY]++;
  return 0;
}

int mock_onCalculateDamage(
    PkCUEngine&, MoveVolatile, PokemonVolatile, PokemonVolatile, uint32_t&) {
  SPDLOG_TRACE("PLUGIN_ON_CALCULATEDAMAGE: mock_onCalculateDamage");
  plugin_calls[PLUGIN_ON_CALCULATEDAMAGE]++;
  return 0;
}

int mock_onEndOfMove(
    PkCUEngine&, MoveVolatile, PokemonVolatile, PokemonVolatile) {
  SPDLOG_TRACE("PLUGIN_ON_ENDOFMOVE: mock_onEndOfMove");
  plugin_calls[PLUGIN_ON_ENDOFMOVE]++;
  return 0;
}

int mock_onModifySecondaryProbability(
    PkCUEngine&, MoveVolatile, PokemonVolatile, PokemonVolatile, FixType&) {
  SPDLOG_TRACE(
      "PLUGIN_ON_MODIFYSECONDARYPROBABILITY: "
      "mock_onModifySecondaryProbability");
  plugin_calls[PLUGIN_ON_MODIFYSECONDARYPROBABILITY]++;
  return 0;
}

int mock_onSecondaryEffect(
    PkCUEngine&, MoveVolatile, PokemonVolatile, PokemonVolatile) {
  SPDLOG_TRACE("PLUGIN_ON_SECONDARYEFFECT: mock_onSecondaryEffect");
  plugin_calls[PLUGIN_ON_SECONDARYEFFECT]++;
  return 0;
}

int mock_onEndOfTurn(PkCUEngine&, PokemonVolatile) {
  SPDLOG_TRACE("PLUGIN_ON_ENDOFTURN: mock_onEndOfTurn");
  plugin_calls[PLUGIN_ON_ENDOFTURN]++;
  return 0;
}

int mock_onEndOfRound(PkCUEngine&, PokemonVolatile) {
  SPDLOG_TRACE("PLUGIN_ON_ENDOFROUND: mock_onEndOfRound");
  plugin_calls[PLUGIN_ON_ENDOFROUND]++;
  return 0;
}

int mock_onSwitchOut(PkCUEngine&, PokemonVolatile) {
  SPDLOG_TRACE("PLUGIN_ON_SWITCHOUT: mock_onSwitchOut");
  plugin_calls[PLUGIN_ON_SWITCHOUT]++;
  return 0;
}

int mock_onSwitchIn(PkCUEngine&, PokemonVolatile) {
  SPDLOG_TRACE("PLUGIN_ON_SWITCHIN: mock_onSwitchIn");
  plugin_calls[PLUGIN_ON_SWITCHIN]++;
  return 0;
}

int mock_onTestMove(
    ConstTeamVolatile,
    ConstPokemonVolatile,
    ConstMoveVolatile,
    const Action&,
    ValidMoveSet&) {
  SPDLOG_TRACE("PLUGIN_ON_TESTMOVE: mock_onTestMove");
  plugin_calls[PLUGIN_ON_TESTMOVE]++;
  return 0;
}

int mock_onTestSwitch(
    ConstPokemonVolatile, ConstPokemonVolatile, const Action&, ValidSwapSet&) {
  SPDLOG_TRACE("PLUGIN_ON_TESTSWITCH: mock_onTestSwitch");
  plugin_calls[PLUGIN_ON_TESTSWITCH]++;
  return 0;
}

int mock_onModifyAction(PkCUEngine&, Action&) {
  SPDLOG_TRACE("PLUGIN_ON_MODIFYACTION: mock_onModifyAction");
  plugin_calls[PLUGIN_ON_MODIFYACTION]++;
  return 0;
}

int mock_onUninitMove(PokemonNonVolatile&, MoveNonVolatile&) {
  SPDLOG_TRACE("PLUGIN_ON_UNINIT: mock_onUninitMove");
  plugin_calls[PLUGIN_ON_UNINIT]++;
  return 0;
}

class MockPokedex : public PokedexStatic {
public:
  MockPokedex() : PokedexStatic(Config(), false) {
    pkdex = this;
    setupTypes();
    setupItems();
    setupAbilities();
    setupNatures();
    setupMoves();
    setupPokemon();
    setupPlugins();
  }

  bool initialize() override { return true; }

 protected:
  void setupTypes() {
    Type tNone; tNone.setName("none");
    Type::no_type = &types_.insert(tNone);

    Type tNormal;
    tNormal.setName("normal");
    Type& t = types_.insert(tNormal);
    t.modTable_[&t] = 1.0 * FPMULTIPLIER;
    t.modTable_[Type::no_type] = 1.0 * FPMULTIPLIER;
  }

  void setupItems() {
    Item iNone; iNone.setName("none");
    Item& iNoneRef = items_.insert(iNone);
    iNoneRef.setHasNoPlugins();
    iNoneRef.lostChild_ = false;
    Item::no_item = &iNoneRef;

    Item it;
    it.setName("test_item");
    Item& i = items_.insert(it);
    i.setHasNoPlugins();
    i.lostChild_ = false;
  }

  void setupAbilities() {
    Ability aNone; aNone.setName("none");
    Ability& aNoneRef = abilities_.insert(aNone);
    aNoneRef.setHasNoPlugins();
    Ability::no_ability = &aNoneRef;

    Ability ab;
    ab.setName("test_ability");
    Ability& a = abilities_.insert(ab);
    a.setHasNoPlugins();
  }

  void setupNatures() {
    Nature nNone; nNone.setName("none");
    Nature::no_nature = &natures_.insert(nNone);
  }

  void setupMoves() {
    const Type* t = &types_.at("normal");
    Move::BuffModArray zeroBuff;
    zeroBuff.fill(0);

    // clang-format off
    Move m_obj(
        "test_move", t, 100, 100, 20, ATK_PHYSICAL, Move::TargetType::ANY_ADJACENT, 0, 10, zeroBuff, zeroBuff, AIL_NV_NONE, AIL_V_NONE, false, "test move");
    m_obj.lostChild = false;
    m_obj.registerPlugin(plugin(pluginCategory::move, "init", PLUGIN_ON_INIT, mock_onInitMove), true);
    moves_.insert(m_obj);

    Move m2_obj(
        "status_move", t, 100, 0, 20, ATK_NODMG, Move::TargetType::ANY_ADJACENT, 0, 0, zeroBuff, zeroBuff, AIL_NV_NONE, AIL_V_NONE, false, "status move");
    m2_obj.lostChild = false;
    m2_obj.registerPlugin(plugin(pluginCategory::move, "init", PLUGIN_ON_INIT, mock_onInitMove), true);
    moves_.insert(m2_obj);
    // clang-format on
  }

  void setupPokemon() {
    const Type* t = &types_.at("normal");
    const Ability* a = &abilities_.at("test_ability");
    const Move* m = &moves_.at("test_move");
    const Move* m2 = &moves_.at("status_move");

    PokemonBase::StatsArray stats = {100, 100, 100, 100, 100, 100};
    PokemonBase::AbilitySet abSet = {a};
    PokemonBase::MoveSet moveSet = {m, m2};

    PokemonBase pb(
        "test_pokemon", {t, Type::no_type}, 100, stats, abSet, moveSet);
    pb.lostChild_ = false;
    pokemon_.insert(pb);

    PokemonBase pb2(
        "test_pokemon2", {t, Type::no_type}, 100, stats, abSet, moveSet);
    pb2.lostChild_ = false;
    pokemon_.insert(pb2);
  }

  void setupPlugins() {
    // Register all plugins
    // clang-format off
    engineExtensions_.registerPlugin(plugin(pluginCategory::engine, "p0", PLUGIN_ON_INIT, mock_onInitMove, 0, current_team), true);
    engineExtensions_.registerPlugin(plugin(pluginCategory::engine, "p1", PLUGIN_ON_RESET, mock_onReset, 0, all_teams), true);
    engineExtensions_.registerPlugin(plugin(pluginCategory::engine, "p2", PLUGIN_ON_SETSPEEDBRACKET, mock_onSetSpeedBracket, 0, all_teams), true);
    engineExtensions_.registerPlugin(plugin(pluginCategory::engine, "p3", PLUGIN_ON_MODIFYSPEED, mock_onModifySpeed, 0, all_teams), true);
    engineExtensions_.registerPlugin(plugin(pluginCategory::engine, "p4", PLUGIN_ON_BEGINNINGOFTURN, mock_onBeginningOfTurn, 0, all_teams), true);
    engineExtensions_.registerPlugin(plugin(pluginCategory::engine, "p5", PLUGIN_ON_EVALUATEMOVE, mock_onEvaluateMove, 0, all_teams), true);
    engineExtensions_.registerPlugin(plugin(pluginCategory::engine, "p6", PLUGIN_ON_SETBASEPOWER, mock_onSetBasePower, 0, all_teams), true);
    engineExtensions_.registerPlugin(plugin(pluginCategory::engine, "p7", PLUGIN_ON_MODIFYBASEPOWER, mock_onModifyBasePower, 0, all_teams), true);
    engineExtensions_.registerPlugin(plugin(pluginCategory::engine, "p8", PLUGIN_ON_MODIFYATTACKPOWER, mock_onModifyAttackPower, 0, all_teams), true);
    engineExtensions_.registerPlugin(plugin(pluginCategory::engine, "p9", PLUGIN_ON_MODIFYCRITICALPOWER, mock_onModifyCriticalPower, 0, all_teams), true);
    engineExtensions_.registerPlugin(plugin(pluginCategory::engine, "p10", PLUGIN_ON_MODIFYRAWDAMAGE, mock_onModifyRawDamage, 0, all_teams), true);
    engineExtensions_.registerPlugin(plugin(pluginCategory::engine, "p11", PLUGIN_ON_SETMOVETYPE, mock_onSetMoveType, 0, all_teams), true);
    engineExtensions_.registerPlugin(plugin(pluginCategory::engine, "p12", PLUGIN_ON_MODIFYSTAB, mock_onModifySTAB, 0, all_teams), true);
    engineExtensions_.registerPlugin(plugin(pluginCategory::engine, "p13", PLUGIN_ON_SETDEFENSETYPE, mock_onSetDefenseType, 0, all_teams), true);
    engineExtensions_.registerPlugin(plugin(pluginCategory::engine, "p14", PLUGIN_ON_MODIFYITEMPOWER, mock_onModifyItemPower, 0, all_teams), true);
    engineExtensions_.registerPlugin(plugin(pluginCategory::engine, "p15", PLUGIN_ON_MODIFYHITPROBABILITY, mock_onModifyHitProbability, 0, all_teams), true);
    engineExtensions_.registerPlugin(plugin(pluginCategory::engine, "p16", PLUGIN_ON_MODIFYCRITPROBABILITY, mock_onModifyCritProbability, 0, all_teams), true);
    engineExtensions_.registerPlugin(plugin(pluginCategory::engine, "p17", PLUGIN_ON_CALCULATEDAMAGE, mock_onCalculateDamage, 0, all_teams), true);
    engineExtensions_.registerPlugin(plugin(pluginCategory::engine, "p18", PLUGIN_ON_ENDOFMOVE, mock_onEndOfMove, 0, all_teams), true);
    engineExtensions_.registerPlugin(plugin(pluginCategory::engine, "p19", PLUGIN_ON_MODIFYSECONDARYPROBABILITY, mock_onModifySecondaryProbability, 0, all_teams), true);
    engineExtensions_.registerPlugin(plugin(pluginCategory::engine, "p20", PLUGIN_ON_SECONDARYEFFECT, mock_onSecondaryEffect, 0, all_teams), true);
    engineExtensions_.registerPlugin(plugin(pluginCategory::engine, "p21", PLUGIN_ON_ENDOFTURN, mock_onEndOfTurn, 0, all_teams), true);
    engineExtensions_.registerPlugin(plugin(pluginCategory::engine, "p22", PLUGIN_ON_ENDOFROUND, mock_onEndOfRound, 0, all_teams), true);
    engineExtensions_.registerPlugin(plugin(pluginCategory::engine, "p23", PLUGIN_ON_SWITCHOUT, mock_onSwitchOut, 0, all_teams), true);
    engineExtensions_.registerPlugin(plugin(pluginCategory::engine, "p24", PLUGIN_ON_SWITCHIN, mock_onSwitchIn, 0, all_teams), true);
    engineExtensions_.registerPlugin(plugin(pluginCategory::engine, "p25", PLUGIN_ON_TESTMOVE, mock_onTestMove, 0, all_teams), true);
    engineExtensions_.registerPlugin(plugin(pluginCategory::engine, "p26", PLUGIN_ON_TESTSWITCH, mock_onTestSwitch, 0, all_teams), true);
    engineExtensions_.registerPlugin(plugin(pluginCategory::engine, "p27", PLUGIN_ON_MODIFYACTION, mock_onModifyAction, 0, all_teams), true);
    engineExtensions_.registerPlugin(plugin(pluginCategory::engine, "p28", PLUGIN_ON_UNINIT, mock_onUninitMove, 0, all_teams), true);
    // clang-format on
  }
};


class EnginePluginTest : public ::testing::Test {
 protected:
  static void resetPluginCalls() { plugin_calls.fill(0); }

  void SetUp() override {
    initialize_logger(spdlog::level::trace);
    resetPluginCalls();
    pokedex_ = std::make_shared<MockPokedex>();
    pokedex_->setAllowInvalidPokemon(true);
    pokedex_->setAllowInvalidTeams(true);

    // clang-format off
    auto team = TeamNonVolatile()
      .addPokemon(PokemonNonVolatile()
        .setBase(pokedex_->pokemon("test_pokemon"))
        .setAbility(pokedex_->ability("test_ability"))
        .setInitialItem(pokedex_->item("test_item"))
        .setNature(pokedex_->nature("none"))
        .addMove(pokedex_->move("test_move"))
        .addMove(pokedex_->move("status_move"))
        .setLevel(100))
      .addPokemon(PokemonNonVolatile()
        .setBase(pokedex_->pokemon("test_pokemon2"))
        .setAbility(pokedex_->ability("test_ability"))
        .setInitialItem(pokedex_->item("test_item"))
        .setNature(pokedex_->nature("none"))
        .addMove(pokedex_->move("test_move"))
        .addMove(pokedex_->move("status_move"))
        .setLevel(100));
    // clang-format on

    environment_ = std::make_shared<EnvironmentNonvolatile>(team, team, true);
    engine_ = std::make_shared<PkCU>();
    engine_->setEnvironment(environment_);
    engine_->setAllowInvalidMoves(true);
  }

  std::shared_ptr<Pokedex> pokedex_;
  std::shared_ptr<EnvironmentNonvolatile> environment_;
  std::shared_ptr<PkCU> engine_;
};


TEST_F(EnginePluginTest, NonvolatileStateInitialization) {
  // environment_->initialize(); // called when when engine_ is initialized
  EXPECT_GT(plugin_calls[PLUGIN_ON_INIT], 0);
}


TEST_F(EnginePluginTest, IsValidActionMove) {
  engine_->isValidAction(engine_->initialState(), Actor(TEAM_A, 0), Action::move(0));
  EXPECT_GT(plugin_calls[PLUGIN_ON_TESTMOVE], 0);
}


TEST_F(EnginePluginTest, IsValidActionSwap) {
  engine_->isValidAction(engine_->initialState(), Actor(TEAM_A, 0), Action::swap(1));
  EXPECT_GT(plugin_calls[PLUGIN_ON_TESTSWITCH], 0);
}


TEST_F(EnginePluginTest, UpdateStateDamagingMove) {
  engine_->updateState(
      engine_->initialState(), Action::move(0), Action::wait());

  EXPECT_GT(plugin_calls[PLUGIN_ON_SETSPEEDBRACKET], 0);
  // EXPECT_GT(plugin_calls[PLUGIN_ON_MODIFYSPEED], 0);
  EXPECT_GT(plugin_calls[PLUGIN_ON_BEGINNINGOFTURN], 0);
  EXPECT_GT(plugin_calls[PLUGIN_ON_MODIFYACTION], 0);
  EXPECT_GT(plugin_calls[PLUGIN_ON_SETBASEPOWER], 0);
  EXPECT_GT(plugin_calls[PLUGIN_ON_MODIFYBASEPOWER], 0);
  EXPECT_GT(plugin_calls[PLUGIN_ON_MODIFYATTACKPOWER], 0);
  EXPECT_GT(plugin_calls[PLUGIN_ON_MODIFYCRITICALPOWER], 0);
  EXPECT_GT(plugin_calls[PLUGIN_ON_MODIFYRAWDAMAGE], 0);
  EXPECT_GT(plugin_calls[PLUGIN_ON_SETMOVETYPE], 0);
  EXPECT_GT(plugin_calls[PLUGIN_ON_MODIFYSTAB], 0);
  EXPECT_GT(plugin_calls[PLUGIN_ON_SETDEFENSETYPE], 0);
  EXPECT_GT(plugin_calls[PLUGIN_ON_MODIFYITEMPOWER], 0);
  EXPECT_GT(plugin_calls[PLUGIN_ON_MODIFYHITPROBABILITY], 0);
  EXPECT_GT(plugin_calls[PLUGIN_ON_MODIFYCRITPROBABILITY], 0);
  EXPECT_GT(plugin_calls[PLUGIN_ON_CALCULATEDAMAGE], 0);
  EXPECT_GT(plugin_calls[PLUGIN_ON_ENDOFMOVE], 0);
  EXPECT_GT(plugin_calls[PLUGIN_ON_MODIFYSECONDARYPROBABILITY], 0);
  EXPECT_GT(plugin_calls[PLUGIN_ON_SECONDARYEFFECT], 0);
  EXPECT_GT(plugin_calls[PLUGIN_ON_ENDOFTURN], 0);
  EXPECT_GT(plugin_calls[PLUGIN_ON_ENDOFROUND], 0);
}


TEST_F(EnginePluginTest, UpdateStateStatusMove) {
  engine_->updateState(
      engine_->initialState(), Action::move(1), Action::wait());

  EXPECT_GT(plugin_calls[PLUGIN_ON_SETSPEEDBRACKET], 0);
  // EXPECT_GT(plugin_calls[PLUGIN_ON_MODIFYSPEED], 0);
  EXPECT_GT(plugin_calls[PLUGIN_ON_BEGINNINGOFTURN], 0);
  EXPECT_GT(plugin_calls[PLUGIN_ON_MODIFYACTION], 0);
  // EXPECT_GT(plugin_calls[PLUGIN_ON_MODIFYHITPROBABILITY], 0);
  EXPECT_GT(plugin_calls[PLUGIN_ON_EVALUATEMOVE], 0);
  EXPECT_GT(plugin_calls[PLUGIN_ON_ENDOFTURN], 0);
  EXPECT_GT(plugin_calls[PLUGIN_ON_ENDOFROUND], 0);
}


TEST_F(EnginePluginTest, UpdateStateSwap) {
  engine_->updateState(
      engine_->initialState(), Action::swap(1), Action::wait());

  // EXPECT_GT(plugin_calls[PLUGIN_ON_SETSPEEDBRACKET], 0);
  // EXPECT_GT(plugin_calls[PLUGIN_ON_BEGINNINGOFTURN], 0);
  // EXPECT_GT(plugin_calls[PLUGIN_ON_MODIFYACTION], 0);
  EXPECT_GT(plugin_calls[PLUGIN_ON_SWITCHOUT], 0);
  EXPECT_GT(plugin_calls[PLUGIN_ON_SWITCHIN], 0);
  EXPECT_GT(plugin_calls[PLUGIN_ON_ENDOFTURN], 0);
  EXPECT_GT(plugin_calls[PLUGIN_ON_ENDOFROUND], 0);
}
