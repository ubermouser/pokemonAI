#ifndef RANKED_H
#define RANKED_H

#include "pkai.h"

#include <vector>
#include <string>
#include <iosfwd>
#include <boost/property_tree/ptree_fwd.hpp>

#include "game.h"
#include "true_skill.h"


struct RankedRecord {
  /* measure of the ranked object's skill */
  TrueSkill skill = TrueSkill{};

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
  mutable bool stateSaved = false;

  RankedRecord() {};

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
    return  ((double)numPlies / (double)numGamesPlayed());
  };

  double winRate() const {
    return double(numWins) / double(numGamesPlayed());
  };
};

class Ranked {
public:
  using Hash = uint64_t;

  static const std::string HEADER;

  virtual ~Ranked() { };
  Ranked(Hash hash=UINT64_MAX) : hash_(hash) {};

  virtual const std::string& getName() const = 0;

  virtual const RankedRecord& record() const { return record_; }
  virtual RankedRecord& record() { return record_; }
  
  virtual const TrueSkill& skill() const { return record().skill; }
  virtual TrueSkill& skill() { return record().skill; }

  virtual void update(const HeatResult& hResult, size_t iTeam);

  virtual std::ostream& print(std::ostream& os) const;
  virtual std::ostream& printStats(std::ostream& os) const;
  virtual boost::property_tree::ptree output(bool printHeader = true) const;
  virtual void input(const boost::property_tree::ptree& tree);

  bool operator<(const Ranked& other) const {
    return skill() > other.skill();
  };

  virtual uint64_t hash() const { return hash_; }

  bool compareHash(uint64_t oHash) const { return hash() == oHash; };
  bool operator==(const Ranked& other) const {return hash() == other.hash(); };
  bool operator!=(const Ranked& other) const { return !(*this == other); };

protected:
  /* generate the hash, and the pokemon subhashes too if true */
  virtual void identify() {
    generateHash(true);
    defineName();
  }
  virtual Hash generateHash(bool generateSubHashes = true) = 0;
  virtual std::string defineName() = 0;

  RankedRecord record_;

  Hash hash_;
};

std::ostream& operator <<(std::ostream& os, const Ranked& tR);

#endif /* RANKED_H */
