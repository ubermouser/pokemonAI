#include "gen4_scripts_internal.h"

namespace gen4 {

void registerGen4Extensions(const Pokedex& pkAI, std::vector<plugin>& extensions) {
  initializePointers(pkAI);

  register_move_alwaysHits(pkAI, extensions);
  register_move_baton_pass(pkAI, extensions);
  register_move_brick_break(pkAI, extensions);
  register_move_counter_mirror_coat(pkAI, extensions);
  register_move_cure_team(pkAI, extensions);
  register_move_destiny_bond(pkAI, extensions);
  register_move_encore(pkAI, extensions);
  register_move_facade(pkAI, extensions);
  register_move_hazards(pkAI, extensions);
  register_move_heal50(pkAI, extensions);
  register_move_hidden_power(pkAI, extensions);
  register_move_highCrit(pkAI, extensions);
  register_move_knock_off(pkAI, extensions);
  register_move_leech_seed(pkAI, extensions);
  register_move_leveled_damage(pkAI, extensions);
  register_move_lifeLeech50(pkAI, extensions);
  register_move_outrage(pkAI, extensions);
  register_move_pain_split(pkAI, extensions);
  register_move_payback(pkAI, extensions);
  register_move_pursuit(pkAI, extensions);
  register_move_rapid_spin(pkAI, extensions);
  register_move_recoil33(pkAI, extensions);
  register_move_rest(pkAI, extensions);
  register_move_roar(pkAI, extensions);
  register_move_screens(pkAI, extensions);
  register_move_substitute(pkAI, extensions);
  register_move_sucker_punch(pkAI, extensions);
  register_move_suicide(pkAI, extensions);
  register_move_struggle(pkAI, extensions);
  register_move_taunt(pkAI, extensions);
  register_move_trap(pkAI, extensions);
  register_move_trick(pkAI, extensions);
  register_move_u_turn(pkAI, extensions);

  register_ability_clear_body(pkAI, extensions);
  register_ability_inner_focus(pkAI, extensions);
  register_ability_intimidate(pkAI, extensions);
  register_ability_levitate(pkAI, extensions);
  register_ability_natural_cure(pkAI, extensions);
  register_ability_no_guard(pkAI, extensions);
  register_ability_pinch_boost(pkAI, extensions);
  register_ability_pressure(pkAI, extensions);
  register_ability_serene_grace(pkAI, extensions);
  register_ability_shadow_tag(pkAI, extensions);
  register_ability_sticky_hold(pkAI, extensions);
  register_ability_technician(pkAI, extensions);
  register_ability_synchronize(pkAI, extensions);

  register_item_choice(pkAI, extensions);
  register_item_focus_sash(pkAI, extensions);
  register_item_leftovers(pkAI, extensions);
  register_item_life_orb(pkAI, extensions);
  register_item_lum_berry(pkAI, extensions);
  register_item_shed_shell(pkAI, extensions);
  register_item_toxic_orb(pkAI, extensions);
  register_item_type_resisting_berry(pkAI, extensions);

  register_engine_common(pkAI, extensions);
}

} // namespace gen4
