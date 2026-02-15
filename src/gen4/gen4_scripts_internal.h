#ifndef GEN4_SCRIPTS_INTERNAL_H
#define GEN4_SCRIPTS_INTERNAL_H

#include <stdint.h>

#include <algorithm>
#include <iostream>
#include <vector>

#include "pokemonai/engine.h"
#include "pokemonai/orphan.h"
#include "pokemonai/pkCU.h"
#include "pokemonai/pkai.h"
#include "pokemonai/pluggable_types.h"
#include "pokemonai/plugin.h"

class Move;
class Item;
class Ability;
class Type;

namespace gen4 {

extern const Pokedex* dex;

extern const Move* absorb_t;
extern const Move* aerialAce_t;
extern const Move* airCutter_t;
extern const Move* aromatherapy_t;
extern const Move* attackOrder_t;
extern const Move* auraSphere_t;
extern const Move* batonPass_t;
extern const Move* bellyDrum_t;
extern const Move* blazeKick_t;
extern const Move* block_t;
extern const Move* braveBird_t;
extern const Move* brickBreak_t;
extern const Move* counter_t;
extern const Move* crabHammer_t;
extern const Move* crossChop_t;
extern const Move* crossPoison_t;
extern const Move* curse_t;
extern const Move* destinyBond_t;
extern const Move* disable_t;
extern const Move* doubleEdge_t;
extern const Move* drainPunch_t;
extern const Move* encore_t;
extern const Move* endeavor_t;
extern const Move* explosion_t;
extern const Move* facade_t;
extern const Move* faintAttack_t;
extern const Move* fissure_t;
extern const Move* flareBlitz_t;
extern const Move* gigaDrain_t;
extern const Move* guillotine_t;
extern const Move* haze_t;
extern const Move* healBell_t;
extern const Move* healOrder_t;
extern const Move* hiddenPower_t;
extern const Move* hornDrill_t;
extern const Move* knockOff_t;
extern const Move* leafBlade_t;
extern const Move* leechLife_t;
extern const Move* leechSeed_t;
extern const Move* lightScreen_t;
extern const Move* magicalLeaf_t;
extern const Move* magnetBomb_t;
extern const Move* meanLook_t;
extern const Move* megaDrain_t;
extern const Move* memento_t;
extern const Move* metalBurst_t;
extern const Move* milkDrink_t;
extern const Move* mirrorCoat_t;
extern const Move* nightShade_t;
extern const Move* nightSlash_t;
extern const Move* outrage_t;
extern const Move* painSplit_t;
extern const Move* payback_t;
extern const Move* perishSong_t;
extern const Move* petalDance_t;
extern const Move* protect_t;
extern const Move* psychoCut_t;
extern const Move* pursuit_t;
extern const Move* rapidSpin_t;
extern const Move* razorLeaf_t;
extern const Move* recover_t;
extern const Move* reflect_t;
extern const Move* refresh_t;
extern const Move* rest_t;
extern const Move* roar_t;
extern const Move* roost_t;
extern const Move* seismicToss_t;
extern const Move* selfDestruct_t;
extern const Move* shadowClaw_t;
extern const Move* shadowPunch_t;
extern const Move* sheerCold_t;
extern const Move* shockWave_t;
extern const Move* slackOff_t;
extern const Move* slash_t;
extern const Move* softBoiled_t;
extern const Move* spiderWeb_t;
extern const Move* spikes_t;
extern const Move* stealthRock_t;
extern const Move* stoneEdge_t;
extern const Move* struggle_t;
extern const Move* substitute_t;
extern const Move* suckerPunch_t;
extern const Move* swift_t;
extern const Move* switcheroo_t;
extern const Move* taunt_t;
extern const Move* thrash_t;
extern const Move* torment_t;
extern const Move* toxicSpikes_t;
extern const Move* triAttack_t;
extern const Move* trick_t;
extern const Move* uTurn_t;
extern const Move* voltTackle_t;
extern const Move* whirlwind_t;
extern const Move* woodHammer_t;

extern const Item* choiceBand_t;
extern const Item* choiceScarf_t;
extern const Item* choiceSpecs_t;
extern const Item* focusSash_t;
extern const Item* leftovers_t;
extern const Item* lifeOrb_t;
extern const Item* lumBerry_t;
extern const Item* shedShell_t;
extern const Item* toxicOrb_t;

extern const Ability* arenaTrap_t;
extern const Ability* blaze_t;
extern const Ability* clearBody_t;
extern const Ability* innerFocus_t;
extern const Ability* intimidate_t;
extern const Ability* levitate_t;
extern const Ability* magnetPull_t;
extern const Ability* naturalCure_t;
extern const Ability* noGuard_t;
extern const Ability* overgrow_t;
extern const Ability* poisonHeal_t;
extern const Ability* pressure_t;
extern const Ability* sereneGrace_t;
extern const Ability* shadowTag_t;
extern const Ability* stickyHold_t;
extern const Ability* sturdy_t;
extern const Ability* swarm_t;
extern const Ability* synchronize_t;
extern const Ability* technician_t;
extern const Ability* torrent_t;

extern const Type* bug_t;
extern const Type* dark_t;
extern const Type* dragon_t;
extern const Type* electric_t;
extern const Type* fighting_t;
extern const Type* fire_t;
extern const Type* flying_t;
extern const Type* ghost_t;
extern const Type* grass_t;
extern const Type* ground_t;
extern const Type* ice_t;
extern const Type* normal_t;
extern const Type* poison_t;
extern const Type* psychic_t;
extern const Type* rock_t;
extern const Type* steel_t;
extern const Type* water_t;

// clang-format off
void initializePointers(const Pokedex& pkAI);

void register_move_alwaysHits(const Pokedex& pkAI, std::vector<plugin>& extensions);
void register_move_baton_pass(const Pokedex& pkAI, std::vector<plugin>& extensions);
void register_move_belly_drum(const Pokedex& pkAI, std::vector<plugin>& extensions);
void register_move_brick_break(const Pokedex& pkAI, std::vector<plugin>& extensions);
void register_move_counter_mirror_coat(const Pokedex& pkAI, std::vector<plugin>& extensions);
void register_move_curse(const Pokedex& pkAI, std::vector<plugin>& extensions);
void register_move_cure_team(const Pokedex& pkAI, std::vector<plugin>& extensions);
void register_move_destiny_bond(const Pokedex& pkAI, std::vector<plugin>& extensions);
void register_move_disable(const Pokedex& pkAI, std::vector<plugin>& extensions);
void register_move_encore(const Pokedex& pkAI, std::vector<plugin>& extensions);
void register_move_endeavor(const Pokedex& pkAI, std::vector<plugin>& extensions);
void register_move_facade(const Pokedex& pkAI, std::vector<plugin>& extensions);
void register_move_hazards(const Pokedex& pkAI, std::vector<plugin>& extensions);
void register_move_haze(const Pokedex& pkAI, std::vector<plugin>& extensions);
void register_move_heal50(const Pokedex& pkAI, std::vector<plugin>& extensions);
void register_move_hidden_power(const Pokedex& pkAI, std::vector<plugin>& extensions);
void register_move_highCrit(const Pokedex& pkAI, std::vector<plugin>& extensions);
void register_move_knock_off(const Pokedex& pkAI, std::vector<plugin>& extensions);
void register_move_leech_seed(const Pokedex& pkAI, std::vector<plugin>& extensions);
void register_move_ohko(const Pokedex& pkAI, std::vector<plugin>& extensions);
void register_move_leveled_damage(const Pokedex& pkAI, std::vector<plugin>& extensions);
void register_move_lifeLeech50(const Pokedex& pkAI, std::vector<plugin>& extensions);
void register_move_metal_burst(const Pokedex& pkAI, std::vector<plugin>& extensions);
void register_move_pain_split(const Pokedex& pkAI, std::vector<plugin>& extensions);
void register_move_payback(const Pokedex& pkAI, std::vector<plugin>& extensions);
void register_move_perish_song(const Pokedex& pkAI, std::vector<plugin>& extensions);
void register_move_protect(const Pokedex& pkAI, std::vector<plugin>& extensions);
void register_move_pursuit(const Pokedex& pkAI, std::vector<plugin>& extensions);
void register_move_rampage(const Pokedex& pkAI, std::vector<plugin>& extensions);
void register_move_rapid_spin(const Pokedex& pkAI, std::vector<plugin>& extensions);
void register_move_recoil33(const Pokedex& pkAI, std::vector<plugin>& extensions);
void register_move_refresh(const Pokedex& pkAI, std::vector<plugin>& extensions);
void register_move_rest(const Pokedex& pkAI, std::vector<plugin>& extensions);
void register_move_roar(const Pokedex& pkAI, std::vector<plugin>& extensions);
void register_move_screens(const Pokedex& pkAI, std::vector<plugin>& extensions);
void register_move_struggle(const Pokedex& pkAI, std::vector<plugin>& extensions);
void register_move_substitute(const Pokedex& pkAI, std::vector<plugin>& extensions);
void register_move_sucker_punch(const Pokedex& pkAI, std::vector<plugin>& extensions);
void register_move_suicide(const Pokedex& pkAI, std::vector<plugin>& extensions);
void register_move_taunt(const Pokedex& pkAI, std::vector<plugin>& extensions);
void register_move_torment(const Pokedex& pkAI, std::vector<plugin>& extensions);
void register_move_trap(const Pokedex& pkAI, std::vector<plugin>& extensions);
void register_move_tri_attack(const Pokedex& pkAI, std::vector<plugin>& extensions);
void register_move_trick(const Pokedex& pkAI, std::vector<plugin>& extensions);
void register_move_u_turn(const Pokedex& pkAI, std::vector<plugin>& extensions);

void register_ability_arena_trap(const Pokedex& pkAI, std::vector<plugin>& extensions);
void register_ability_clear_body(const Pokedex& pkAI, std::vector<plugin>& extensions);
void register_ability_inner_focus(const Pokedex& pkAI, std::vector<plugin>& extensions);
void register_ability_intimidate(const Pokedex& pkAI, std::vector<plugin>& extensions);
void register_ability_levitate(const Pokedex& pkAI, std::vector<plugin>& extensions);
void register_ability_magnet_pull(const Pokedex& pkAI, std::vector<plugin>& extensions);
void register_ability_natural_cure(const Pokedex& pkAI, std::vector<plugin>& extensions);
void register_ability_no_guard(const Pokedex& pkAI, std::vector<plugin>& extensions);
void register_ability_pinch_boost(const Pokedex& pkAI, std::vector<plugin>& extensions);
void register_ability_poison_heal(const Pokedex& pkAI, std::vector<plugin>& extensions);
void register_ability_pressure(const Pokedex& pkAI, std::vector<plugin>& extensions);
void register_ability_serene_grace(const Pokedex& pkAI, std::vector<plugin>& extensions);
void register_ability_shadow_tag(const Pokedex& pkAI, std::vector<plugin>& extensions);
void register_ability_sticky_hold(const Pokedex& pkAI, std::vector<plugin>& extensions);
void register_ability_sturdy(const Pokedex& pkAI, std::vector<plugin>& extensions);
void register_ability_synchronize(const Pokedex& pkAI, std::vector<plugin>& extensions);
void register_ability_technician(const Pokedex& pkAI, std::vector<plugin>& extensions);

void register_item_choice(const Pokedex& pkAI, std::vector<plugin>& extensions);
void register_item_focus_sash(const Pokedex& pkAI, std::vector<plugin>& extensions);
void register_item_leftovers(const Pokedex& pkAI, std::vector<plugin>& extensions);
void register_item_life_orb(const Pokedex& pkAI, std::vector<plugin>& extensions);
void register_item_lum_berry(const Pokedex& pkAI, std::vector<plugin>& extensions);
void register_item_shed_shell(const Pokedex& pkAI, std::vector<plugin>& extensions);
void register_item_toxic_orb(const Pokedex& pkAI, std::vector<plugin>& extensions);
void register_item_type_resisting_berry(const Pokedex& pkAI, std::vector<plugin>& extensions);

void register_engine_common(const Pokedex& pkAI, std::vector<plugin>& extensions);

int trapped_by_ability_common(
    ConstPokemonVolatile cPKV,
    ConstPokemonVolatile tPKV,
    bool isGroundedOnly,
    bool isSteelOnly,
    const Ability* trappingAbility,
    ValidSwapSet& switchAllowed);

void registerGen4Extensions(const Pokedex& pkAI, std::vector<plugin>& extensions);
// clang-format on

} // namespace gen4

#endif // GEN4_SCRIPTS_INTERNAL_H
