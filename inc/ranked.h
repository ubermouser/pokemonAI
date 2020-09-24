#ifndef RANKED_H
#define RANKED_H

#include "pkai.h"

#include <vector>
#include <string>
#include <ostream>

#include "true_skill.h"


struct RankedRecord {
  /* measure of the ranked object's skill */
  TrueSkill skill = TrueSkill{};

  /* the generation that this ranked object was created */
  size_t generation = 0;

  /* total number of wins by this ranked object */
  uint64_t numWins = 0;

  /* total number of losses by this ranked object */
  uint64_t numLosses = 0;

  /* total number of draws by this ranked object */
  uint64_t numDraws = 0;

  /* total number of ties by this ranked object */
  uint64_t numTies = 0;

  /* total number of plies played by this ranked object */
  uint64_t numPlies = 0;

  /* has state been written to disk? */
  bool stateSaved = false;

  void resetRecord() {
    numWins = 0;
    numTies = 0;
    numDraws = 0;
    numLosses = 0;
    numPlies = 0;
  };

  uint64_t numGamesPlayed() const {
    return numWins + numTies + numDraws + numLosses;
  };

  double pliesPerGame() const {
    auto numPlayed = numGamesPlayed();
    if (numPlayed == 0) { return 0.0; }
    return  ((double)numPlies() / (double)numGamesPlayed());
  };
};

class Ranked {
public:
  using Hash = uint64_t;

  static const std::string header;

  virtual ~Ranked() { };
  Ranked(size_t generation = 0);

  virtual const std::string& getName() const = 0;

  const RankedRecord& record() const { return record_; }
  RankedRecord& record() { return record_; }
  
  const TrueSkill& skill() const { return record().skill; }
  TrueSkill& skill() { return record().skill; }

  bool operator<(const Ranked& other) const
  {
    return skill() > other.skill();
  };

  virtual uint64_t hash() const { return hash_; }

  bool compareHash(uint64_t oHash) const
  {
    return hash() == oHash;
  };

  bool operator==(const Ranked& other) const
  {
    return hash() == other.hash();
  };

  bool operator!=(const Ranked& other) const
  {
    return !(*this == other);
  };

protected:
  /* generate the hash, and the pokemon subhashes too if true */
  void generateHash(bool generateSubHashes = true) = 0;
  void defineName();

  RankedRecord record_;

  Hash hash_;
};

#endif /* RANKED_H */
