//#define PKAI_STATIC
#include "pokemonai/pokedex.h"
//#undef PKAI_STATIC

#include <vector>
#include <iostream>
#include <sstream>
#include <string>
#include <boost/format.hpp>
#include <boost/property_tree/ptree.hpp>

#include "pokemonai/engine.h"
#include "pokemonai/game.h"
#include "pokemonai/ranked_team.h"
#include "pokemonai/init_toolbox.h"

const std::string RankedTeam::HEADER = "PKAIR1";
namespace pt = boost::property_tree;


RankedTeam::RankedTeam(
    const TeamNonVolatile& cTeam,
    PokemonLeague& league,
    const Contribution& contribution)
  : Ranked(cTeam.hash()),
    nv_(cTeam),
    ranked_components_(findSubteams(league)),
    contribution_(contribution),
    synergy_(TrueSkill::synergy()) {
  identify();
  computeSkill();
}


std::vector<RankedPokemonPtr> RankedTeam::findSubteams(PokemonLeague& league) const {
  std::vector<RankedPokemonPtr> result;

  for (size_t iTeammate = 0; iTeammate != nv_.getNumTeammates(); ++iTeammate) {
    RankedPokemon pkmn{nv_.teammate(iTeammate)};
    if (league.count(pkmn.hash()) == 0) {
      // add to league as it does not exist
      league[pkmn.hash()] = std::make_shared<RankedPokemon>(pkmn);
    }
    result.push_back(league.at(pkmn.hash()));
  }

  return result;
}


std::vector<GroupContribution> RankedTeam::contributions() {
  std::vector<GroupContribution> result; result.reserve(teammates().size() + 1);

  result.push_back({synergy_, contribution_.synergy, hash()}); // team synergy
  // individual pokemon
  for (RankedPokemonPtr& pokemon: teammates()) {
    result.push_back({pokemon->skill(), contribution_.pokemon, pokemon->hash()});
  }
  return result;
}


TrueSkill& RankedTeam::computeSkill() {
  skill() = TrueSkill::combine(contributions());
  return skill();
}


void RankedTeam::identify() {
  Hash newHash = nv().hash();
  if (newHash != hash()) {
    generateHash(true);
    defineName();
  }
}


pt::ptree RankedTeam::output(bool printHeader) const {
  pt::ptree result;
  // header:
  if (printHeader) {
    result.put("header", HEADER);
  };

  result.put_child("record", Ranked::output());
  result.put("hash", hash());
  
  result.put_child("team", nv_.output(printHeader));

  return result;
} // endOf outputRankedTeam


void RankedTeam::input(const pt::ptree& tree) {
  /*
   * Header data:
   * PKAIR<VERSION
   * <team hash>\n
   * <pokemon 0 hash> <pokemon 1 hash> ...\n
   * <numPlies 1> <numPlies 1> <numPlies 2> ..\n
   * <move 0.0> <move 0.1> <move 0.2> ...\n
   * <numWins> <numLosses> <numTies> <numDraws>\n
   * STANDARD TEAM REMAINS
   */
  Ranked::input(tree.get_child("record"));
  hash_ = tree.get<RankedTeam::Hash>("hash");

  nv_.input(tree.get_child("team"));

  PokemonLeague dummyLeague;
  findSubteams(dummyLeague);
} // endOf inputTeam (ranked)


RankedTeam::Hash RankedTeam::generateHash(bool generateSubHashes) {
  hash_ = nv_.hash();
  return nv_.hash();
}


void RankedTeam::update(const HeatResult& result, size_t iTeam) {
  Ranked::update(result, iTeam);

  for (size_t iPokemon = 0; iPokemon != ranked_components_.size(); ++iPokemon) {
    ranked_components_[iPokemon]->update(result, iTeam, iPokemon);
  }
} //endOf update


LeagueCount TeamLeague::countTeamLeague() const {
  LeagueCount result; result.fill(0);
  for (auto& team: *this) {
    result[team.second->nv().getNumTeammates() - 1] += 1;
  }

  return result;
}


std::vector<RankedTeamPtr> TeamLeague::getLeague(size_t numPokemon) const {
  std::vector<RankedTeamPtr> result;
  for (auto& team: *this) {
    if (team.second->nv().getNumTeammates() != numPokemon) { continue; }
    result.push_back(team.second);
  }

  return result;
}


std::vector<RankedTeamPtr> TeamLeague::getAll() const {
  std::vector<RankedTeamPtr> result; result.reserve(size());
  for (auto& team: *this) { result.push_back(team.second); }

  return result;
}
