#include "pokemonai/ranked.h"

#include <fmt/format.h>
#include <fmt/ostream.h>

#include <algorithm>
#include <boost/property_tree/json_parser.hpp>
#include <boost/property_tree/ptree.hpp>
#include <iostream>
#include <numeric>

#include "pokemonai/game.h"
#include "pokemonai/true_skill.h"

namespace pt = boost::property_tree;
const std::string Ranked::HEADER = "PKART0";


const std::string& Ranked::getName() const
{
  static const std::string unnamed("-UNNAMED RANKED OBJ-");
  return unnamed;
};

void Ranked::update(const HeatResult& hResult, size_t iTeam) {
  auto& rec = record();
  rec.stateSaved = false;
  rec.numPlies += uint64_t(hResult.numPlies * hResult.matchesPlayed);
  rec.numWins += hResult.score[iTeam];
  rec.numLosses += hResult.score[(iTeam + 1) % 2];
  rec.numTies +=
      hResult.matchesPlayed - std::accumulate(hResult.score.begin(), hResult.score.end(), 0);
} // endOf update


std::ostream& Ranked::print(std::ostream& os) const {
  os << fmt::format("{:<34.34} ", getName());
  printStats(os);

  return os;
}


std::ostream& Ranked::printStats(std::ostream& os) const {
  os << fmt::format(
      "{} w={:4.2f} g={}",
      fmt::streamed(skill()),
      record().winRate(),
      record().numGamesPlayed());
  return os;
}

pt::ptree Ranked::output(bool printHeader) const {
  pt::ptree result;
  // header:
  if (printHeader) {
    result.put("header", HEADER);
  }

  result.put("numWins", record().numWins);
  result.put("numLosses", record().numLosses);
  result.put("numDraws", record().numDraws);
  result.put("numTies", record().numTies);
  result.put("numPlies", record().numPlies);
  // trueSkill:
  result.put_child("skill", record().skill.output());

  return result;
};

void Ranked::input(const pt::ptree& tree) {
  auto header = tree.get<std::string>("header");
  if (header != HEADER) {
    SPDLOG_CRITICAL("Ranked header mismatch! Should be \"{}\", was \"{}\"!", 
        HEADER, header);
  }

  record().numWins = tree.get<uint64_t>("numWins");
  record().numLosses = tree.get<uint64_t>("numLosses");
  record().numDraws = tree.get<uint64_t>("numDraws");
  record().numTies = tree.get<uint64_t>("numTies");
  record().numPlies = tree.get<uint64_t>("numPlies");
  // trueskill:
  skill().input(tree.get_child("skill"));
};

std::ostream& operator <<(std::ostream& os, const Ranked& tR) {
  tR.print(os);
  return os;
};