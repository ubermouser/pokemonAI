#include "gen4_scripts_internal.h"

namespace gen4 {


int move_roar_forceSwitch(
    PkCUEngine& cu,
    const Actor& actor,
    const Action& action,
    const Actor& target) {
  TeamVolatile tTV = cu.getTTV();

  // Find all valid switch-ins
  std::vector<size_t> validSwitchIns;
  for (size_t i = 0; i < tTV.nv().getNumTeammates(); ++i) {
    if (i == tTV.getICPKV()) continue; // Current pokemon
    if (tTV.teammate(i).isAlive()) { validSwitchIns.push_back(i); }
  }

  if (validSwitchIns.empty()) {
    // Move fails if no one to switch to
    return 0;
  }

  auto targetActor = cu.getTarget();
  auto& frame = cu.getStackFrame();

  size_t iTargetActor = cu.getActorIndex(targetActor);
  size_t numOptions = validSwitchIns.size();
  size_t originalIBase = cu.getIBase();
  size_t currentEnvIndex = originalIBase;

  for (size_t i = 0; i < numOptions; ++i) {
    size_t envIndex;
    if (i < numOptions - 1) {
      std::array<size_t, 2> splitResult;
      FixType prob = FixType(1) / (int32_t)(numOptions - i);
      cu.duplicateState(splitResult, prob, currentEnvIndex);
      envIndex = splitResult[1];
      currentEnvIndex = splitResult[0];
    } else {
      envIndex = currentEnvIndex;
    }

    auto& branchFrame = cu.getStackFrame(envIndex);
    branchFrame.iActor = iTargetActor;
    branchFrame.actions[targetActor] = Action::swap(validSwitchIns[i]);
    branchFrame.targets[targetActor] = {
        Actor(targetActor.iTeam(), validSwitchIns[i])};

    // Mark that the original actor has finished their turn to avoid re-execution
    // if targetActor context swap loops back.
    branchFrame.actions[actor] = Action::wait();

    cu.gotoStackStage(envIndex, StageType::PRESWITCH);
  }

  return 2;
}

void register_move_roar(const Pokedex& pkAI, std::vector<plugin>& extensions) {
  // clang-format off
  extensions.push_back(pluginOnEvaluateMove(move, "roar", move_roar_forceSwitch, 0, current_team));
  extensions.push_back(pluginOnEvaluateMove(move, "whirlwind", move_roar_forceSwitch, 0, current_team));
  // clang-format on
}

} // namespace gen4
