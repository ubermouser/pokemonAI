#include "gen4_scripts_internal.h"

namespace gen4 {

int move_perishSong_set(
    PkCUEngine& cu,
    MoveVolatile mV,
    PokemonVolatile cPKV,
    PokemonVolatile tPKV) {
  if (&mV.getBase() != perishSong_t) { return 0; }

  // Perish Song affects all active Pokemon (user and target in 1v1)
  // Logic: Set perishSong counter to 3 if not already set.
  // Note: Soundproof ability blocks this, but we skip it as per plan/constraints.

  // Set for user
  if (cPKV.status().cTeammate.perishSong == 0) {
    cPKV.status().cTeammate.perishSong = 3;
  }

  // Set for target
  if (tPKV.status().cTeammate.perishSong == 0) {
    tPKV.status().cTeammate.perishSong = 3;
  }

  return 1;
}

int move_perishSong_update(PkCUEngine& cu, PokemonVolatile cPKV) {
  auto& teamStatus = cPKV.status().cTeammate;
  if (teamStatus.perishSong == 0) { return 0; }

  // Decrement counter
  teamStatus.perishSong--;

  // If counter reached 0, faint
  if (teamStatus.perishSong == 0) {
    cPKV.setHP(0);
  }

  return 1;
}

void register_move_perish_song(const Pokedex& pkAI, std::vector<plugin>& extensions) {
  extensions.push_back(plugin(move, "perish song", PLUGIN_ON_EVALUATEMOVE, move_perishSong_set, 0, current_team));
  extensions.push_back(plugin(move, "perish song", PLUGIN_ON_ENDOFROUND, move_perishSong_update, 0, all_teams));
}

} // namespace gen4
