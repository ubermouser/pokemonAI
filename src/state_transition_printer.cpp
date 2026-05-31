#include "pokemonai/state_transition_printer.h"

#include <fmt/color.h>
#include <fmt/format.h>
#include <fmt/ostream.h>

#include <iostream>
#include <regex>
#include <sstream>

#include "pokemonai/item.h"
#include "pokemonai/team_status.h"
#include "pokemonai/type.h"


std::string StateTransitionPrinter::printString(
    const ConstEnvironmentVolatile& osP,
    const ConstEnvironmentPossible& nsP,
    bool withStyle) {
  std::stringstream ss;
  print(ss, osP, nsP);
  std::string result = ss.str();
  if (!withStyle) { return stripControlCharacters(result); }
  return result;
}


std::string StateTransitionPrinter::stripControlCharacters(
    const std::string& input) {
  static const std::regex ansi_regex("\x1b\\[[0-9;]*[mK]");
  return std::regex_replace(input, ansi_regex, "");
}

void StateTransitionPrinter::print(
    std::ostream& os,
    const ConstEnvironmentVolatile& osP,
    const ConstEnvironmentPossible& nsP) {
  // Determine order of actions based on movesFirst bit
  std::array<size_t, 2> order = {0, 1};
  if (nsP.flagsFor(TEAM_B).isMovedFirst()) {
    order = {1, 0};
  }

  for (size_t iTeam : order) {
    const auto& teamOld = osP.getTeam(iTeam);
    for (const auto& [actor, pkOld] : teamOld.yieldPokemon()) {
      reportSwitch(os, nsP, actor);
      if (pkOld.isActive() && nsP.teammate(actor).isActive()) {
        reportHitResult(os, osP, nsP, actor);
      }
    }

    for (const auto& [actor, pkOld] : teamOld.yieldPokemon()) {
      const auto& pkNew = nsP.teammate(actor);
      if (pkOld.isActive() && !pkNew.isActive()) {
        reportStatusChange(os, pkOld, pkNew);
      }
    }
  }

  // General HP and Status changes comparison between oldState and newState
  for (size_t iTeam = 0; iTeam < 2; ++iTeam) {
    const auto& teamOld = osP.getTeam(iTeam);
    const auto& teamNew = nsP.getTeam(iTeam);

    reportFainting(os, osP, nsP, iTeam);

    for (const auto& [actor, pkOld] : teamOld.yieldPokemon()) {
      const auto& pkNew = teamNew.teammate(actor);

      if (pkNew.isActive() || (pkOld.isActive() && !pkNew.isAlive())) {
        reportDamage(os, pkOld, pkNew);
        reportItemUsage(os, pkOld, pkNew);
        reportStatusChange(os, pkOld, pkNew);
        reportVolatileStatusChange(os, pkOld, pkNew);
        reportStatBoosts(os, pkOld, pkNew);
      }
    }

    // Report team volatile status changes once using the first active pokemon on the team
    for (const auto& [actor, pkNew] : teamNew.yieldActivePokemon()) {
      reportTeamVolatileStatusChange(os, teamOld, teamNew, actor.iTeammate());
      break;
    }
  }
}


void StateTransitionPrinter::reportSwitch(
    std::ostream& os, const ConstEnvironmentPossible& nsP, const Actor& actor) {
  if (nsP.flagsFor(actor).isSwitched()) {
    fmt::print(
        os,
        "Team {} sent out {}!\n",
        (actor.iTeam() == 0 ? "A" : "B"),
        pokemonName(nsP.teammate(actor)));
  }
}


void StateTransitionPrinter::reportHitResult(
    std::ostream& os,
    const ConstEnvironmentVolatile& osP,
    const ConstEnvironmentPossible& nsP,
    const Actor& actor) {
  auto pkName = pokemonName(osP.teammate(actor));

  auto pkProxy = nsP.flagsFor(actor);

  if (pkProxy.isWaited()) {
    // Waiting, nothing to report here usually
  } else if (pkProxy.isFreeMove()) {
    // Free turn, usually for multi-turn moves or something
  } else if (pkProxy.isBlocked()) {
    fmt::print(os, "{}'s move was blocked!\n", pkName);
  } else if (!pkProxy.isSwitched()) {
    if (!pkProxy.isHit()) {
      fmt::print(os, "{}'s attack missed!\n", pkName);
    } else {
      if (pkProxy.isCrit()) {
        os << fmt::format(
            fmt::fg(fmt::color::gold) | fmt::emphasis::bold,
            "{} scored a critical hit!\n", pkName);
      }
      if (pkProxy.isSecondary()) {
        // secondary effect triggered
      }
    }
  }
}


void StateTransitionPrinter::reportFainting(
    std::ostream& os,
    const ConstEnvironmentVolatile& osP,
    const ConstEnvironmentPossible& nsP,
    size_t iTeam) {
  const auto& teamOld = osP.getTeam(iTeam);
  const auto& teamNew = nsP.getTeam(iTeam);
  for (size_t i = 0; i < teamOld.nv().getNumTeammates(); ++i) {
    const auto& pkOld = teamOld.teammate(i);
    const auto& pkNew = teamNew.teammate(i);
    if (!pkNew.isAlive() && pkOld.isAlive()) {
      os << fmt::format(
          fmt::fg(fmt::color::red) | fmt::emphasis::bold,
          "{} fainted!\n",
          pkOld.nv().getName());
    }
  }
}


void StateTransitionPrinter::reportDamage(
    std::ostream& os,
    const ConstPokemonVolatile& pkOld,
    const ConstPokemonVolatile& pkNew) {
  if (pkNew.getHP() < pkOld.getHP()) {
    uint32_t damage = pkOld.getHP() - pkNew.getHP();
    float percent = (float)damage * 100.0f / pkNew.nv().getMaxHP();
    os << fmt::format(
        "{} lost {} HP ({:.1f}%)!\n", pokemonName(pkNew), damage, percent);
  } else if (pkNew.getHP() > pkOld.getHP()) {
    os << fmt::format("{} restored HP!\n", pokemonName(pkNew));
  }
}


void StateTransitionPrinter::reportStatusChange(
    std::ostream& os,
    const ConstPokemonVolatile& pkOld,
    const ConstPokemonVolatile& pkNew) {
  if (pkNew.getStatusAilment() != pkOld.getStatusAilment()) {
    std::string statusMsg = "";
    fmt::color col = fmt::color::white;
    switch (pkNew.getStatusAilment()) {
    case AIL_NV_BURN:
      statusMsg = "was burned!";
      col = fmt::color::orange_red;
      break;
    case AIL_NV_FREEZE:
      statusMsg = "was frozen solid!";
      col = fmt::color::light_sky_blue;
      break;
    case AIL_NV_PARALYSIS:
      statusMsg = "is paralyzed! It may be unable to move!";
      col = fmt::color::yellow;
      break;
    case AIL_NV_POISON:
      statusMsg = "was poisoned!";
      col = fmt::color::orchid;
      break;
    case AIL_NV_POISON_TOXIC:
      statusMsg = "was badly poisoned!";
      col = fmt::color::purple;
      break;
    case AIL_NV_SLEEP_1T:
    case AIL_NV_SLEEP_2T:
    case AIL_NV_SLEEP_3T:
    case AIL_NV_SLEEP_4T:
    case AIL_NV_REST_1T:
    case AIL_NV_REST_2T:
    case AIL_NV_REST_3T:
      statusMsg = "fell asleep!";
      col = fmt::color::light_gray;
      break;
    case AIL_NV_NONE:
      statusMsg = "woke up / was cured!";
      break;
    }
    if (!statusMsg.empty()) {
      os << fmt::format(
          "{} {}\n",
          pkNew.nv().getName(),
          fmt::format(fmt::fg(col), "{}", statusMsg));
    }
  }
}


void StateTransitionPrinter::reportStatBoosts(
    std::ostream& os,
    const ConstPokemonVolatile& pkOld,
    const ConstPokemonVolatile& pkNew) {
  const auto& bOld = pkOld.status().boosts;
  const auto& bNew = pkNew.status().boosts;
  auto pkName = pokemonName(pkNew);

  auto reportBoost = [&](int8_t oldB, int8_t newB, const char* name) {
    if (newB > oldB) {
      fmt::print(os, "{}'s {} rose!\n", pkName, name);
    } else if (newB < oldB) {
      fmt::print(os, "{}'s {} fell!\n", pkName, name);
    }
  };
  reportBoost(bOld.B_ATK, bNew.B_ATK, "Attack");
  reportBoost(bOld.B_DEF, bNew.B_DEF, "Defense");
  reportBoost(bOld.B_SPA, bNew.B_SPA, "Sp. Atk");
  reportBoost(bOld.B_SPD, bNew.B_SPD, "Sp. Def");
  reportBoost(bOld.B_SPE, bNew.B_SPE, "Speed");
  reportBoost(bOld.B_ACC, bNew.B_ACC, "Accuracy");
  reportBoost(bOld.B_EVA, bNew.B_EVA, "Evasiveness");
}


void StateTransitionPrinter::reportVolatileStatusChange(
    std::ostream& os,
    const ConstPokemonVolatile& pkOld,
    const ConstPokemonVolatile& pkNew) {
  const auto& vOld = pkOld.status();
  const auto& vNew = pkNew.status();
  auto pkName = pokemonName(pkNew);

  // Pokemon-specific Volatile statuses
  if (vNew.confused > 0 && vOld.confused == 0) {
    fmt::print(os, "{} became confused!\n", pkName);
  }
  if (vNew.infatuate > 0 && vOld.infatuate == 0) {
    fmt::print(os, "{} fell in love!\n", pkName);
  }
  if (vNew.leechSeed > 0 && vOld.leechSeed == 0) {
    fmt::print(os, "{} was seeded!\n", pkName);
  }
  if (vNew.taunt_duration > 0 && vOld.taunt_duration == 0) {
    fmt::print(os, "{} was taunted!\n", pkName);
  }
  if (vNew.lockIn_duration > 0 && vOld.lockIn_duration == 0) {
    fmt::print(os, "{} is locked in!\n", pkName);
  }
  if (vNew.substitute > 0 && vOld.substitute == 0) {
    fmt::print(os, "{} put up a substitute!\n", pkName);
  } else if (vNew.substitute == 0 && vOld.substitute > 0) {
    fmt::print(os, "{}'s substitute faded!\n", pkName);
  }
}


void StateTransitionPrinter::reportTeamVolatileStatusChange(
    std::ostream& os,
    const ConstTeamVolatile& teamOld,
    const ConstTeamVolatile& teamNew,
    size_t activeIndex) {
  const auto& nvOld = teamOld.status();
  const auto& nvNew = teamNew.status();
  auto teamId = pokemonName(teamNew.teammate(activeIndex));

  if (nvNew.stealthRock > 0 && nvOld.stealthRock == 0) {
    fmt::print(os, "Pointed stones float in the air around {}!\n", teamId);
  }
  if (nvNew.spikes > nvOld.spikes) {
    fmt::print(os, "Spikes were scattered around {}'s feet!\n", teamId);
  }
  if (nvNew.toxicSpikes > nvOld.toxicSpikes) {
    fmt::print(os, "Toxic spikes were scattered around {}'s feet!\n", teamId);
  }
  if (nvNew.lightScreen > 0 && nvOld.lightScreen == 0) {
    fmt::print(
        os,
        "Light Screen made {}'s team stronger against special attacks!\n",
        teamId);
  }
  if (nvNew.reflect > 0 && nvOld.reflect == 0) {
    fmt::print(
        os,
        "Reflect made {}'s team stronger against physical attacks!\n",
        teamId);
  }
}


std::string StateTransitionPrinter::pokemonName(
    const ConstPokemonVolatile& pk) {
  return fmt::format(fmt::fg(fmt::color::cyan), "{}", pk.nv().getName());
}


void StateTransitionPrinter::reportItemUsage(
    std::ostream& os,
    const ConstPokemonVolatile& pkOld,
    const ConstPokemonVolatile& pkNew) {
  if (pkOld.hasItem() && !pkNew.hasItem()) {
    fmt::print(
        os, "{} used its {}!\n", pokemonName(pkNew), pkOld.getItem().getName());
  }
}
