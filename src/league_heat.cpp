#include "pokemonai/league_heat.h"

#include <unordered_map>
#include <string>

#include "pokemonai/nature.h"
#include "pokemonai/type.h"

void LeagueHeat::calculateContribution() {
  for (const auto& game : games) {
    const auto& hr = game.heatResult;
    for (size_t iTeam = 0; iTeam < 2; ++iTeam) {
      const auto& bg = (iTeam == 0) ? game.team_a : game.team_b;
      const auto& nvTeam = bg->team().nv();
      const auto& hrt = hr.teams[iTeam];
      for (size_t iPkmn = 0; iPkmn < nvTeam.getNumTeammates(); ++iPkmn) {
        const auto& pkhr = hrt.pokemon[iPkmn];
        const auto& pknv = nvTeam.teammate(iPkmn);
        auto& pkStat = counts.pokemon[pknv.getBase().getName()];
        pkStat.aggregateContribution += pkhr.aggregateContribution;
        pkStat.simpleContribution += pkhr.simpleContribution;
        pkStat.participation += pkhr.participation;

        for (size_t iMove = 0; iMove < pknv.getNumMoves(); ++iMove) {
          auto& moveStat = counts.moves[pknv.getMove_base(iMove).getName()];
          moveStat.aggregateContribution +=
              pkhr.aggregateContribution * pkhr.moveUse[iMove];
          moveStat.simpleContribution +=
              pkhr.simpleContribution * pkhr.moveUse[iMove];
          moveStat.participation += pkhr.participation * pkhr.moveUse[iMove];
        }
      }
    }
  }
}


void LeagueHeat::calculateCounts() {
  for (auto& bgPair : battlegroups) {
    const auto& team = bgPair.second->team().nv();
    for (size_t iTeammate = 0; iTeammate < team.getNumTeammates();
         ++iTeammate) {
      const auto& pokemon = team.teammate(iTeammate);
      const auto& base = pokemon.getBase();
      counts.pokemon[base.getName()].count++;
      if (&pokemon.getNature() != Nature::no_nature) {
        counts.natures[pokemon.getNature().getName()].count++;
      }
      if (pokemon.abilityExists()) {
        counts.abilities[pokemon.getAbility().getName()].count++;
      }
      if (pokemon.hasInitialItem()) {
        counts.items[pokemon.getInitialItem().getName()].count++;
      }

      for (size_t iType = 0; iType < 2; ++iType) {
        if (&base.getType(iType) != Type::no_type) {
          counts.types[base.getType(iType).getName()].count++;
        }
      }

      for (size_t iMove = 0; iMove < pokemon.getNumMoves(); ++iMove) {
        counts.moves[pokemon.getMove_base(iMove).getName()].count++;
      }
    }
  }
}
