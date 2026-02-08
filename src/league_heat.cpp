#include "pokemonai/league_heat.h"

#include <fmt/format.h>

#include <algorithm>
#include <cmath>
#include <string>
#include <unordered_map>

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
void LeagueHeat::calculateUsage() {
  // First pass: aggregate skill statistics from the league
  std::unordered_map<std::string, std::vector<double>> skillsBySpecies;
  for (auto& pair : pokemon) {
    skillsBySpecies[pair.second->get().getBase().getName()].push_back(
        pair.second->skill().rank());
  }

  for (auto& pair : skillsBySpecies) {
    auto& usage = pokemonUsage[pair.first];
    double sum = 0;
    double maxSkill = -1e9;
    for (double s : pair.second) {
      sum += s;
      maxSkill = std::max(maxSkill, s);
    }
    usage.avgSkill = sum / pair.second.size();
    usage.maximumSkill = maxSkill;
  }

  for (auto& game : games) {
    const auto& nv = *game.heatResult.nv;
    for (size_t iTeam = 0; iTeam < 2; ++iTeam) {
      const auto& team = nv.getTeam(iTeam);
      const auto& opponentTeam = nv.getTeam(1 - iTeam);
      const auto& teamResult = game.heatResult.teams[iTeam];

      for (size_t iPkmn = 0; iPkmn < team.getNumTeammates(); ++iPkmn) {
        const auto& pkmn = team.teammate(iPkmn);
        const std::string& name = pkmn.getBase().getName();
        auto& usage = pokemonUsage[name];

        usage.count++;
        if (pkmn.abilityExists()) {
          usage.abilities[pkmn.getAbility().getName()]++;
        } else {
          usage.abilities["none"]++;
        }

        if (pkmn.hasInitialItem()) {
          usage.items[pkmn.getInitialItem().getName()]++;
        } else {
          usage.items["none"]++;
        }

        // EVs: HP/Atk/Def/SpA/SpD/Spe
        std::string spread = fmt::format(
            "{}:{}/{}/{}/{}/{}/{}",
            pkmn.getNature().getName(),
            pkmn.getEV(FV_HITPOINTS),
            pkmn.getEV(FV_ATTACK),
            pkmn.getEV(FV_DEFENSE),
            pkmn.getEV(FV_SPATTACK),
            pkmn.getEV(FV_SPDEFENSE),
            pkmn.getEV(FV_SPEED));
        usage.spreads[spread]++;

        for (size_t iMove = 0; iMove < pkmn.getNumMoves(); ++iMove) {
          usage.moves[pkmn.getMove_base(iMove).getName()] +=
              teamResult.pokemon[iPkmn].moveUse[iMove];
        }

        for (size_t iTeammate = 0; iTeammate < team.getNumTeammates();
             ++iTeammate) {
          if (iPkmn == iTeammate) continue;
          usage.teammates[team.teammate(iTeammate).getBase().getName()]++;
        }

        const auto& encounters = teamResult.pokemon[iPkmn].encounters;
        for (size_t iOpponent = 0; iOpponent < opponentTeam.getNumTeammates();
             ++iOpponent) {
          const auto& enc = encounters[iOpponent];
          const std::string& opponentName =
              opponentTeam.teammate(iOpponent).getBase().getName();
          auto& uEnc = usage.encounters[opponentName];
          uEnc.numKOs += enc.numKOs;
          uEnc.numSwitches += enc.numSwitches;
          uEnc.numTotal += enc.numTotal;
        }
      }
    }
  }

  // Finalize statistics
  for (auto& pair : pokemonUsage) {
    auto& usage = pair.second;
    for (auto& ePair : usage.encounters) {
      auto& e = ePair.second;
      if (e.numTotal > 0) {
        double k = (double)(e.numKOs + e.numSwitches);
        double n = (double)e.numTotal;
        e.mean = k / n;
        e.stddev = std::sqrt(e.mean * (1.0 - e.mean) / n);
        e.score = 100.0 * (e.mean - 4.0 * e.stddev);
      }
    }
  }
}

std::string LeagueHeat::produceDatasheet(const std::string& pokemonName) const {
  if (pokemonUsage.count(pokemonName) == 0) return "No data for " + pokemonName;
  const auto& usage = pokemonUsage.at(pokemonName);

  std::string out;
  out += "+----------------------------------------+\n";
  out += fmt::format("| {:<38} |\n", pokemonName);
  out += "+----------------------------------------+\n";
  out += fmt::format("| Raw count: {:<27} |\n", usage.count);
  out += fmt::format("| Avg. Skill: {:<26.1f} |\n", usage.avgSkill);
  out += fmt::format("| Max. Skill: {:<26.1f} |\n", usage.maximumSkill);
  out += "+----------------------------------------+\n";

  auto printSection =
      [&](const std::string& title, const auto& map, bool isDouble = false) {
        out += fmt::format("| {:<38} |\n", title);
        std::vector<std::pair<std::string, double>> sorted;
        double total = 0;
        for (auto& p : map) {
          sorted.push_back({p.first, (double)p.second});
          total += (double)p.second;
        }
        std::sort(sorted.begin(), sorted.end(), [](auto& a, auto& b) {
          return b.second > a.second;
        });

        for (size_t i = 0; i < std::min(sorted.size(), size_t(12)); ++i) {
          double pct = 100.0 * sorted[i].second / usage.count;
          out += fmt::format("| {:<29} {:>7.3f}% |\n", sorted[i].first, pct);
        }
        out += "+----------------------------------------+\n";
      };

  // Abilities
  printSection("Abilities", usage.abilities);
  // Items
  printSection("Items", usage.items);
  // Spreads
  printSection("Spreads", usage.spreads);
  // Moves (moves are already normalized per game in GameResult, so we need to
  // divide by game count of this pokemon)
  {
    out += "| Moves                                  |\n";
    std::vector<std::pair<std::string, double>> sorted;
    for (auto& p : usage.moves) sorted.push_back(p);
    std::sort(sorted.begin(), sorted.end(), [](auto& a, auto& b) {
      return b.second > a.second;
    });
    for (size_t i = 0; i < std::min(sorted.size(), size_t(12)); ++i) {
      out += fmt::format(
          "| {:<29} {:>7.3f}% |\n",
          sorted[i].first,
          100.0 * sorted[i].second / usage.count);
    }
    out += "+----------------------------------------+\n";
  }
  // Teammates
  printSection("Teammates", usage.teammates);

  // Checks and Counters
  out += "| Checks and Counters                    |\n";
  std::vector<std::pair<std::string, const PokemonUsageStats::EncounterStats*>>
      sortedEnc;
  for (auto& p : usage.encounters) sortedEnc.push_back({p.first, &p.second});
  std::sort(sortedEnc.begin(), sortedEnc.end(), [](auto& a, auto& b) {
    return b.second->score > a.second->score;
  });

  for (size_t i = 0; i < std::min(sortedEnc.size(), size_t(12)); ++i) {
    const auto& name = sortedEnc[i].first;
    const auto& e = *sortedEnc[i].second;
    if (e.numTotal == 0) continue;
    out += fmt::format(
        "| {:<12.12} {:>6.2f} ({:>5.2f}±{:>4.2f}){:>6} |\n",
        name,
        e.score,
        100.0 * e.mean,
        100.0 * e.stddev,
        "");
    double koPct = 100.0 * e.numKOs / e.numTotal;
    double swPct = 100.0 * e.numSwitches / e.numTotal;
    out += fmt::format(
        "| {:^38} |\n",
        fmt::format("({:>4.1f}% KOed / {:>4.1f}% switched out)", koPct, swPct));
  }
  out += "+----------------------------------------+\n";

  return out;
}
