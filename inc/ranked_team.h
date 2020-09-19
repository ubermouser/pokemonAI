#ifndef TEAM_RANKED_H
#define TEAM_RANKED_H

#include "../inc/pkai.h"

#include <stdint.h>
#include <array>
#include <vector>

#include "ranked.h"

#include "true_skill.h"

#include "../inc/team_nonvolatile.h"

class Pokedex;
class Game;


class RankedTeam : public Ranked
{
public:
  /* team that this ranked_team represents */
  TeamNonVolatile team;

private:
  /* hashes of team_nonvolatile object and the pokemon in the array */
  std::array<uint64_t, 7> hash;

  /* sum of the each pokemon's rank from game results */
  std::array<uint32_t, 6> rankPoints;

  /* number of plies a given pokemon has been active in */
  std::array<uint32_t, 6> numPlies;
  
  /* number of times a given move has been used */
  std::array< std::array<uint32_t, 5>, 6> numMoves;

  void resetRecord()
  {
    Ranked::resetRecord();
    
    rankPoints.fill(0);
    numPlies.fill(0);
    
    for (size_t iTeammate = 0; iTeammate != team.getMaxNumTeammates(); ++iTeammate)
    {
      numMoves[iTeammate].fill(0);
    }
  };

public:
  static const std::string header;

  ~RankedTeam() { };
  RankedTeam(const TeamNonVolatile& cTeam = TeamNonVolatile(), size_t generation = 0, const trueSkillSettings& settings = trueSkillSettings::defaultSettings);
  RankedTeam(const RankedTeam& other);

  /* define the names of the team and its pokemon given the hash */
  void defineNames();

  const std::string& getName() const { return team.getName(); };

  /* generate the hash, and the pokemon subhashes too if true */
  void generateHash(bool generateSubHashes = true);

  uint64_t getHash() const
  {
    return hash[0];
  };

  uint64_t getTeammateHash(size_t iTeammate) const
  {
    return hash[iTeammate + 1];
  };

  uint32_t getTeamRankPoints() const
  {
    uint32_t total = 0;
    for (size_t iTeammate = 0; iTeammate != team.getMaxNumTeammates(); ++iTeammate)
    {
      total += rankPoints[iTeammate];
    }
    return total;
  };

  uint32_t getNumRankPoints(size_t iTeammate) const
  {
    return rankPoints[iTeammate];
  };

  uint32_t getNumMovesUsed(size_t iTeammate, size_t iMove) const
  {
    return numMoves[iTeammate][iMove];
  };

  uint32_t getTeammatePlies(size_t iTeammate) const
  {
    return numPlies[iTeammate];
  };

  uint32_t getNumPlies() const
  {
    uint32_t total = 0;
    for (size_t iTeammate = 0; iTeammate != team.getMaxNumTeammates(); ++iTeammate)
    {
      total += numPlies[iTeammate];
    }
    return total;
  };

  void output(std::ostream& oFile, bool printHeader = true) const;

  bool input(const std::vector<std::string>& lines, size_t& firstLine);

private:

  static PokemonNonVolatile createRandom_single(const TeamNonVolatile& cTeam, size_t iReplace = SIZE_MAX);

  /* randomize a pokemon's species */
  static void randomSpecies(const TeamNonVolatile& cTeam, PokemonNonVolatile& cPokemon, size_t iReplace = SIZE_MAX);

  static void randomAbility(PokemonNonVolatile& cPokemon);

  static void randomNature(PokemonNonVolatile& cPokemon);

  static void randomItem(PokemonNonVolatile& cPokemon);

  /* randomize a number of the pokemon's IVs */
  static void randomIV(PokemonNonVolatile& cPokemon, size_t numIVs = 1);

  static void randomEV(PokemonNonVolatile& cPokemon);

  static void randomGender(PokemonNonVolatile& cPokemon);

  /* randomize a number of the pokemon's moves */
  static void randomMove(PokemonNonVolatile& cPokemon, size_t numMoves = 1);

  static void mutate_single(RankedTeam& cRankteam, size_t iTeammate, size_t numMutations);

  static PokemonNonVolatile crossover_single(
    const PokemonNonVolatile& parentA, 
    const PokemonNonVolatile& parentB);

  /* select a new team based on roulette selection; the team selected will consist of at or less than numPokemon teammates */
  static std::array<size_t, 2> selectRandom_single(
    const RankedTeam& existing,
    const std::array<std::vector<RankedTeam>, 6>& league, 
    size_t numPokemon,
    bool allowLess = true);

public:

  /* generate a random child, initialize its descriptive variables, define its hash */
  static RankedTeam createRandom(
    const trueSkillSettings& settings,
    size_t numPokemon, 
    size_t _generation);

  static TeamNonVolatile createRandom(size_t numPokemon);

  /* update two team rankings */
  size_t update(const Game& cGame, const TrueSkillTeam& cTeam, size_t iTeam);

  /* generate a child through sexual reproduction of two ranked_team objects with the same number of teammates */
  static RankedTeam crossover(
    const trueSkillSettings& settings,
    const RankedTeam& parentA, 
    const RankedTeam& parentB);

  /* generate a child through asexual reproduction of two ranked objects */
  static RankedTeam mutate(
    const trueSkillSettings& settings,
    const std::array<std::vector<RankedTeam>, 6>& league, 
    const RankedTeam& parent, 
    size_t numMutations = 1);

  /* generate a new team based on roulette selection from smaller leagues */
  static RankedTeam selectRandom(
    const trueSkillSettings& settings,
    const std::array<std::vector<RankedTeam>, 6>& league, 
    size_t numPokemon);

  friend class PokemonAI;
  friend class PkIO;
  friend std::ostream& operator <<(std::ostream& os, const RankedTeam& tR);
};

std::ostream& operator <<(std::ostream& os, const RankedTeam& tR);

#endif /* TEAM_RANKED_H */
