#include "gen4_scripts_internal.h"

namespace gen4 {

int item_choiceScarf_modSpeed(
    PkCUEngine& cu, const Actor& actor, uint32_t& speed) {
  PokemonVolatile cPKV = cu.getPKV(actor);
  if (&cPKV.getItem() != choiceScarf_t) { return 0; }

  speed = (speed * 3) / 2;  // increase speed by 50%
  return 1;
}

int item_choiceItem_modPower(
    PkCUEngine& cu,
    const Actor& actor,
    const Action& action,
    const Actor& target,
    fpType& modifier) {
  PokemonVolatile cPKV = cu.getPKV(actor);
  MoveVolatile mV = cu.getMV(actor);
  const Item* cItem = &cPKV.getItem();
  if ((cItem != choiceBand_t) && (cItem != choiceSpecs_t)) { return 0; }

  auto damageType = mV.getBase().getDamageType();
  if ((cItem == choiceBand_t && damageType == ATK_PHYSICAL) ||
      (cItem == choiceSpecs_t && damageType == ATK_SPECIAL)) {
    modifier *= 1.5;
  }

  return 1;
};

int item_choiceItem_lockMove(PkCUEngine& cu, const Actor& actor) {
  PokemonVolatile cPKV = cu.getPKV(actor);
  const Item* cItem = &cPKV.getItem();
  if ((cItem != choiceBand_t) && (cItem != choiceScarf_t) &&
      (cItem != choiceSpecs_t)) {
    return 0;
  }

  // action is guaranteed to be a move action:
  size_t action_idx = cu.getCAction().iMove() + 1;
  // the pokemon may not use another move until it switches out:
  cPKV.status().itemScratch = action_idx;

  return 1;
}

int item_choiceItem_testLockedMove(
    ConstTeamVolatile cTV,
    ConstPokemonVolatile cPKV,
    ConstMoveVolatile mV,
    const Action& action,
    ValidMoveSet& moveAllowed) {
  const Item* cItem = &cPKV.getItem();
  if ((cItem != choiceBand_t) && (cItem != choiceScarf_t) &&
      (cItem != choiceSpecs_t)) {
    return 0;
  }

  size_t choice_item_idx = cPKV.status().itemScratch;

  // if the user has not used a move with their choice item yet:
  if (choice_item_idx == 0) { return 1; }

  // else, if the choice item has chosen a move, the only acceptable move is the
  // choice move:
  size_t action_idx = action.iMove() + 1;
  moveAllowed[VALID_MOVE_SCRIPT] =
      moveAllowed[VALID_MOVE_SCRIPT] & (choice_item_idx == action_idx);

  return 1;
}

void register_item_choice(const Pokedex& pkAI, std::vector<plugin>& extensions) {
  extensions.push_back(pluginOnBeginningOfTurn(item, "choice band", item_choiceItem_lockMove, 0, all_teams));
  extensions.push_back(pluginOnTestMove(item, "choice band", item_choiceItem_testLockedMove, 0, all_teams));
  extensions.push_back(pluginOnModifyRawDamage(item, "choice band", item_choiceItem_modPower, 0, all_teams));
  extensions.push_back(pluginOnBeginningOfTurn(item, "choice scarf", item_choiceItem_lockMove, 0, all_teams));
  extensions.push_back(pluginOnTestMove(item, "choice scarf", item_choiceItem_testLockedMove, 0, all_teams));
  extensions.push_back(pluginOnModifySpeed(item, "choice scarf", item_choiceScarf_modSpeed, 0, all_teams));
  extensions.push_back(pluginOnBeginningOfTurn(item, "choice specs", item_choiceItem_lockMove, 0, all_teams));
  extensions.push_back(pluginOnTestMove(item, "choice specs", item_choiceItem_testLockedMove, 0, all_teams));
  extensions.push_back(pluginOnModifyRawDamage(item, "choice specs", item_choiceItem_modPower, 0, all_teams));
}

} // namespace gen4
