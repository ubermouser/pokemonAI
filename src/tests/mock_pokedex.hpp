#ifndef MOCK_POKEDEX_HPP
#define MOCK_POKEDEX_HPP

#include <spdlog/spdlog.h>
#include <array>
#include <memory>
#include <string>
#include <vector>

#include "pokemonai/engine.h"
#include "pokemonai/pkCU.h"
#include "pokemonai/pokedex_static.h"
#include "pokemonai/pokemon_nonvolatile.h"
#include "pokemonai/team_nonvolatile.h"

// Global pointer needed by some classes
extern PKAISHARED const Pokedex* pkdex;

static std::array<int, PLUGIN_MAXSIZE> plugin_calls;

// Mock plugin functions
inline int mock_onInitMove(PokemonNonVolatile&, MoveNonVolatile&) {
  SPDLOG_TRACE("PLUGIN_ON_INIT: mock_onInitMove");
  plugin_calls[PLUGIN_ON_INIT]++;
  return 0;
}

inline int mock_onReset(PkCUEngine&, void*) {
  SPDLOG_TRACE("PLUGIN_ON_RESET: mock_onReset");
  plugin_calls[PLUGIN_ON_RESET]++;
  return 0;
}

inline int mock_onSetSpeedBracket(
    PkCUEngine&, MoveVolatile, PokemonVolatile, int32_t&) {
  SPDLOG_TRACE("PLUGIN_ON_SETSPEEDBRACKET: mock_onSetSpeedBracket");
  plugin_calls[PLUGIN_ON_SETSPEEDBRACKET]++;
  return 0;
}

inline int mock_onModifySpeed(PkCUEngine&, PokemonVolatile, uint32_t&) {
  SPDLOG_TRACE("PLUGIN_ON_MODIFYSPEED: mock_onModifySpeed");
  plugin_calls[PLUGIN_ON_MODIFYSPEED]++;
  return 0;
}

inline int mock_onBeginningOfTurn(PkCUEngine&, PokemonVolatile) {
  SPDLOG_TRACE("PLUGIN_ON_BEGINNINGOFTURN: mock_onBeginningOfTurn");
  plugin_calls[PLUGIN_ON_BEGINNINGOFTURN]++;
  return 0;
}

inline int mock_onEvaluateMove(
    PkCUEngine&, MoveVolatile, PokemonVolatile, PokemonVolatile) {
  SPDLOG_TRACE("PLUGIN_ON_EVALUATEMOVE: mock_onEvaluateMove");
  plugin_calls[PLUGIN_ON_EVALUATEMOVE]++;
  return 0;
}

inline int mock_onSetBasePower(
    PkCUEngine&, MoveVolatile, PokemonVolatile, PokemonVolatile, uint32_t&) {
  SPDLOG_TRACE("PLUGIN_ON_SETBASEPOWER: mock_onSetBasePower");
  plugin_calls[PLUGIN_ON_SETBASEPOWER]++;
  return 0;
}

inline int mock_onModifyBasePower(
    PkCUEngine&, MoveVolatile, PokemonVolatile, PokemonVolatile, fpType&) {
  SPDLOG_TRACE("PLUGIN_ON_MODIFYBASEPOWER: mock_onModifyBasePower");
  plugin_calls[PLUGIN_ON_MODIFYBASEPOWER]++;
  return 0;
}

inline int mock_onModifyAttackPower(
    PkCUEngine&, MoveVolatile, PokemonVolatile, PokemonVolatile, fpType&) {
  SPDLOG_TRACE("PLUGIN_ON_MODIFYATTACKPOWER: mock_onModifyAttackPower");
  plugin_calls[PLUGIN_ON_MODIFYATTACKPOWER]++;
  return 0;
}

inline int mock_onModifyCriticalPower(
    PkCUEngine&, MoveVolatile, PokemonVolatile, PokemonVolatile, fpType&) {
  SPDLOG_TRACE("PLUGIN_ON_MODIFYCRITICALPOWER: mock_onModifyCriticalPower");
  plugin_calls[PLUGIN_ON_MODIFYCRITICALPOWER]++;
  return 0;
}

inline int mock_onModifyRawDamage(
    PkCUEngine&, MoveVolatile, PokemonVolatile, PokemonVolatile, fpType&) {
  SPDLOG_TRACE("PLUGIN_ON_MODIFYRAWDAMAGE: mock_onModifyRawDamage");
  plugin_calls[PLUGIN_ON_MODIFYRAWDAMAGE]++;
  return 0;
}

inline int mock_onSetMoveType(
    PkCUEngine&, MoveVolatile, PokemonVolatile, PokemonVolatile, const Type*&) {
  SPDLOG_TRACE("PLUGIN_ON_SETMOVETYPE: mock_onSetMoveType");
  plugin_calls[PLUGIN_ON_SETMOVETYPE]++;
  return 0;
}

inline int mock_onModifySTAB(
    PkCUEngine&, MoveVolatile, PokemonVolatile, PokemonVolatile, fpType&) {
  SPDLOG_TRACE("PLUGIN_ON_MODIFYSTAB: mock_onModifySTAB");
  plugin_calls[PLUGIN_ON_MODIFYSTAB]++;
  return 0;
}

inline int mock_onSetDefenseType(
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

inline int mock_onModifyItemPower(
    PkCUEngine&, MoveVolatile, PokemonVolatile, PokemonVolatile, fpType&) {
  SPDLOG_TRACE("PLUGIN_ON_MODIFYITEMPOWER: mock_onModifyItemPower");
  plugin_calls[PLUGIN_ON_MODIFYITEMPOWER]++;
  return 0;
}

inline int mock_onModifyHitProbability(
    PkCUEngine&, MoveVolatile, PokemonVolatile, PokemonVolatile, FixType&) {
  SPDLOG_TRACE("PLUGIN_ON_MODIFYHITPROBABILITY: mock_onModifyHitProbability");
  plugin_calls[PLUGIN_ON_MODIFYHITPROBABILITY]++;
  return 0;
}

inline int mock_onModifyCritProbability(
    PkCUEngine&, MoveVolatile, PokemonVolatile, PokemonVolatile, FixType&) {
  SPDLOG_TRACE("PLUGIN_ON_MODIFYCRITPROBABILITY: mock_onModifyCritProbability");
  plugin_calls[PLUGIN_ON_MODIFYCRITPROBABILITY]++;
  return 0;
}

inline int mock_onCalculateDamage(
    PkCUEngine&, MoveVolatile, PokemonVolatile, PokemonVolatile, uint32_t&) {
  SPDLOG_TRACE("PLUGIN_ON_CALCULATEDAMAGE: mock_onCalculateDamage");
  plugin_calls[PLUGIN_ON_CALCULATEDAMAGE]++;
  return 0;
}

inline int mock_onEndOfMove(
    PkCUEngine&, MoveVolatile, PokemonVolatile, PokemonVolatile) {
  SPDLOG_TRACE("PLUGIN_ON_ENDOFMOVE: mock_onEndOfMove");
  plugin_calls[PLUGIN_ON_ENDOFMOVE]++;
  return 0;
}

inline int mock_onModifySecondaryProbability(
    PkCUEngine&, MoveVolatile, PokemonVolatile, PokemonVolatile, FixType&) {
  SPDLOG_TRACE(
      "PLUGIN_ON_MODIFYSECONDARYPROBABILITY: "
      "mock_onModifySecondaryProbability");
  plugin_calls[PLUGIN_ON_MODIFYSECONDARYPROBABILITY]++;
  return 0;
}

inline int mock_onSecondaryEffect(
    PkCUEngine&, MoveVolatile, PokemonVolatile, PokemonVolatile) {
  SPDLOG_TRACE("PLUGIN_ON_SECONDARYEFFECT: mock_onSecondaryEffect");
  plugin_calls[PLUGIN_ON_SECONDARYEFFECT]++;
  return 0;
}

inline int mock_onEndOfTurn(PkCUEngine&, PokemonVolatile) {
  SPDLOG_TRACE("PLUGIN_ON_ENDOFTURN: mock_onEndOfTurn");
  plugin_calls[PLUGIN_ON_ENDOFTURN]++;
  return 0;
}

inline int mock_onEndOfRound(PkCUEngine&, PokemonVolatile) {
  SPDLOG_TRACE("PLUGIN_ON_ENDOFROUND: mock_onEndOfRound");
  plugin_calls[PLUGIN_ON_ENDOFROUND]++;
  return 0;
}

inline int mock_onSwitchOut(PkCUEngine&, PokemonVolatile) {
  SPDLOG_TRACE("PLUGIN_ON_SWITCHOUT: mock_onSwitchOut");
  plugin_calls[PLUGIN_ON_SWITCHOUT]++;
  return 0;
}

inline int mock_onSwitchIn(PkCUEngine&, PokemonVolatile) {
  SPDLOG_TRACE("PLUGIN_ON_SWITCHIN: mock_onSwitchIn");
  plugin_calls[PLUGIN_ON_SWITCHIN]++;
  return 0;
}

inline int mock_onTestMove(
    ConstTeamVolatile,
    ConstPokemonVolatile,
    ConstMoveVolatile,
    const Action&,
    ValidMoveSet&) {
  SPDLOG_TRACE("PLUGIN_ON_TESTMOVE: mock_onTestMove");
  plugin_calls[PLUGIN_ON_TESTMOVE]++;
  return 0;
}

inline int mock_onTestSwitch(
    ConstPokemonVolatile, ConstPokemonVolatile, const Action&, ValidSwapSet&) {
  SPDLOG_TRACE("PLUGIN_ON_TESTSWITCH: mock_onTestSwitch");
  plugin_calls[PLUGIN_ON_TESTSWITCH]++;
  return 0;
}

inline int mock_onModifyAction(PkCUEngine&, Action&) {
  SPDLOG_TRACE("PLUGIN_ON_MODIFYACTION: mock_onModifyAction");
  plugin_calls[PLUGIN_ON_MODIFYACTION]++;
  return 0;
}

inline int mock_onUninitMove(PokemonNonVolatile&, MoveNonVolatile&) {
  SPDLOG_TRACE("PLUGIN_ON_UNINIT: mock_onUninitMove");
  plugin_calls[PLUGIN_ON_UNINIT]++;
  return 0;
}

// Special testing plugins
inline int mock_onExplosion(
    PkCUEngine&, MoveVolatile mV, PokemonVolatile user, PokemonVolatile target) {
  if (&mV.getBase() != &pkdex->move("move_explosion")) { return 0; }

  SPDLOG_TRACE(
      "Fainting user {} and target {}",
      user.nv().getName(),
      target.nv().getName());
  user.setHP(0);
  target.setHP(0);
  return 2;
}

inline int mock_onFaint(
    PkCUEngine&, MoveVolatile mV, PokemonVolatile, PokemonVolatile target) {
  if (&mV.getBase() != &pkdex->move("move_faint")) { return 0; }

  SPDLOG_TRACE("Fainting target {}", target.nv().getName());
  target.setHP(0);
  return 2;
}

inline int mock_onSuicide(
    PkCUEngine&, MoveVolatile mV, PokemonVolatile user, PokemonVolatile) {
  if (&mV.getBase() != &pkdex->move("move_suicide")) { return 0; }

  SPDLOG_TRACE("Fainting user {}", user.nv().getName());
  user.setHP(0);
  return 2;
}

inline int mock_zeroPP(
    PkCUEngine&, MoveVolatile mV, PokemonVolatile, PokemonVolatile target) {
  if (&mV.getBase() != &pkdex->move("move_zero_pp")) { return 0; }

  SPDLOG_TRACE("Zeroing PP of target {}", target.nv().getName());
  for (auto [iMove, move] : target.yieldMoves()) { move.setPP(0); }
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
    // Standard moves for plugin tests
    moves_.insert(Move("test_move", t, 100, 100, 20, ATK_PHYSICAL, Move::ANY_ADJACENT, 0, 10, zeroBuff, zeroBuff, AIL_NV_NONE, AIL_V_NONE, true, "test move"));
    moves_.insert(Move("status_move", t, 100, 0, 20, ATK_NODMG, Move::ANY_ADJACENT, 0, 0, zeroBuff, zeroBuff, AIL_NV_NONE, AIL_V_NONE, true, "status move"));

    // Moves for every TargetType
    moves_.insert(Move("move_self", t, 100, 0, 20, ATK_NODMG, Move::SELF, 0, 0, zeroBuff, zeroBuff, AIL_NV_NONE, AIL_V_NONE, false, "self"));
    moves_.insert(Move("move_any_adjacent", t, 100, 100, 20, ATK_PHYSICAL, Move::ANY_ADJACENT, 0, 0, zeroBuff, zeroBuff, AIL_NV_NONE, AIL_V_NONE, false, "any adjacent"));
    moves_.insert(Move("move_any_adjacent_ally", t, 100, 0, 20, ATK_NODMG, Move::ANY_ADJACENT_ALLY, 0, 0, zeroBuff, zeroBuff, AIL_NV_NONE, AIL_V_NONE, false, "any adjacent ally"));
    moves_.insert(Move("move_any_adjacent_enemy", t, 100, 100, 20, ATK_PHYSICAL, Move::ANY_ADJACENT_ENEMY, 0, 0, zeroBuff, zeroBuff, AIL_NV_NONE, AIL_V_NONE, false, "any adjacent enemy"));
    moves_.insert(Move("move_any_adjacent_ally_self", t, 100, 0, 20, ATK_NODMG, Move::ANY_ADJACENT_ALLY_SELF, 0, 0, zeroBuff, zeroBuff, AIL_NV_NONE, AIL_V_NONE, false, "any adjacent ally self"));
    moves_.insert(Move("move_any_active", t, 100, 100, 20, ATK_PHYSICAL, Move::ANY_ACTIVE, 0, 0, zeroBuff, zeroBuff, AIL_NV_NONE, AIL_V_NONE, false, "any active"));
    moves_.insert(Move("move_any_ally", t, 100, 0, 20, ATK_NODMG, Move::ANY_ALLY, 0, 0, zeroBuff, zeroBuff, AIL_NV_NONE, AIL_V_NONE, false, "any ally"));
    moves_.insert(Move("move_all_adjacent", t, 100, 100, 20, ATK_PHYSICAL, Move::ALL_ADJACENT, 0, 0, zeroBuff, zeroBuff, AIL_NV_NONE, AIL_V_NONE, false, "all adjacent"));
    moves_.insert(Move("move_all_adjacent_enemy", t, 100, 100, 20, ATK_PHYSICAL, Move::ALL_ADJACENT_ENEMY, 0, 0, zeroBuff, zeroBuff, AIL_NV_NONE, AIL_V_NONE, false, "all adjacent enemy"));
    moves_.insert(Move("move_all_adjacent_ally", t, 100, 0, 20, ATK_NODMG, Move::ALL_ADJACENT_ALLY, 0, 0, zeroBuff, zeroBuff, AIL_NV_NONE, AIL_V_NONE, false, "all adjacent ally"));
    moves_.insert(Move("move_all_active_allies", t, 100, 0, 20, ATK_NODMG, Move::ALL_ACTIVE_ALLIES, 0, 0, zeroBuff, zeroBuff, AIL_NV_NONE, AIL_V_NONE, false, "all active allies"));
    moves_.insert(Move("move_all_active_enemies", t, 100, 0, 20, ATK_NODMG, Move::ALL_ACTIVE_ENEMIES, 0, 0, zeroBuff, zeroBuff, AIL_NV_NONE, AIL_V_NONE, false, "all active enemies"));
    moves_.insert(Move("move_all_active", t, 100, 0, 20, ATK_NODMG, Move::ALL_ACTIVE, 0, 0, zeroBuff, zeroBuff, AIL_NV_NONE, AIL_V_NONE, false, "all active"));
    moves_.insert(Move("move_side_ally", t, 100, 0, 20, ATK_NODMG, Move::SIDE_ALLY, 0, 0, zeroBuff, zeroBuff, AIL_NV_NONE, AIL_V_NONE, false, "side ally"));
    moves_.insert(Move("move_side_enemy", t, 100, 0, 20, ATK_NODMG, Move::SIDE_ENEMY, 0, 0, zeroBuff, zeroBuff, AIL_NV_NONE, AIL_V_NONE, false, "side enemy"));
    moves_.insert(Move("move_side_all", t, 100, 0, 20, ATK_NODMG, Move::SIDE_ALL, 0, 0, zeroBuff, zeroBuff, AIL_NV_NONE, AIL_V_NONE, false, "side all"));
    moves_.insert(Move("move_all_allies", t, 100, 0, 20, ATK_NODMG, Move::ALL_ALLIES, 0, 0, zeroBuff, zeroBuff, AIL_NV_NONE, AIL_V_NONE, false, "all allies"));
    moves_.insert(Move("move_all_enemies", t, 100, 0, 20, ATK_NODMG, Move::ALL_ENEMIES, 0, 0, zeroBuff, zeroBuff, AIL_NV_NONE, AIL_V_NONE, false, "all enemies"));
    moves_.insert(Move("move_all_field", t, 100, 0, 20, ATK_NODMG, Move::ALL_FIELD, 0, 0, zeroBuff, zeroBuff, AIL_NV_NONE, AIL_V_NONE, false, "all field"));

    // Fainting moves
    moves_.insert(Move("move_explosion", t, 100, 0, 20, ATK_NODMG, Move::ALL_ADJACENT, 0, 0, zeroBuff, zeroBuff, AIL_NV_NONE, AIL_V_NONE, true, "explosion"));
    moves_.insert(Move("move_faint", t, 100, 0, 20, ATK_NODMG, Move::ANY_ADJACENT, 0, 0, zeroBuff, zeroBuff, AIL_NV_NONE, AIL_V_NONE, true, "faint"));
    moves_.insert(Move("move_suicide", t, 100, 0, 20, ATK_NODMG, Move::SELF, 0, 0, zeroBuff, zeroBuff, AIL_NV_NONE, AIL_V_NONE, true, "suicide"));
    moves_.insert(Move("move_zero_pp", t, 100, 0, 20, ATK_NODMG, Move::ANY_ADJACENT, 0, 0, zeroBuff, zeroBuff, AIL_NV_NONE, AIL_V_NONE, true, "zero pp"));

    // Move plugins
    moves_.at("test_move").registerPlugin(plugin(pluginCategory::move, "init", PLUGIN_ON_INIT, mock_onInitMove), true);
    moves_.at("status_move").registerPlugin(plugin(pluginCategory::move, "init", PLUGIN_ON_INIT, mock_onInitMove), true);
    moves_.at("move_explosion").registerPlugin(plugin(pluginCategory::move, "explosion", PLUGIN_ON_EVALUATEMOVE, mock_onExplosion), true);
    moves_.at("move_faint").registerPlugin(plugin(pluginCategory::move, "faint", PLUGIN_ON_EVALUATEMOVE, mock_onFaint), true);
    moves_.at("move_suicide").registerPlugin(plugin(pluginCategory::move, "suicide", PLUGIN_ON_EVALUATEMOVE, mock_onSuicide), true);
    moves_.at("move_zero_pp").registerPlugin(plugin(pluginCategory::move, "zero_pp", PLUGIN_ON_EVALUATEMOVE, mock_zeroPP), true);
    // clang-format on
  }

  void setupPokemon() {
    const Type* t = &types_.at("normal");
    const Ability* a = &abilities_.at("test_ability");
    
    PokemonBase::StatsArray stats = {100, 100, 100, 100, 100, 100};
    PokemonBase::AbilitySet abSet = {a};
    
    // Pokemon with subsets of moves
    auto addPkmn = [&](std::string name) {
      PokemonBase::MoveSet moveSet;
      for (const auto& [name, move] : moves_) { moveSet.insert(&move); }
      PokemonBase pb(name, {t, Type::no_type}, 100, stats, abSet, moveSet);
      pb.lostChild_ = false;
      pokemon_.insert(pb);
    };

    addPkmn("test_pokemon");
    addPkmn("test_pokemon2");
    addPkmn("test_pokemon3");
    addPkmn("test_pokemon4");
  }

  void setupPlugins() {
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

#endif // MOCK_POKEDEX_HPP
