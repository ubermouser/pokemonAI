#ifndef RANKED_H
#define RANKED_H

#include "../inc/pkai.h"

#include <vector>
#include <string>

#include "../inc/true_skill.h"

class Game;
class TrueSkillTeam;

class Ranked {
protected:
  /* measure of the ranked object's skill */
  TrueSkill skill;

  /* the generation that this ranked object was created */
  uint32_t generation;

  /* total number of wins by this ranked object */
  uint32_t numWins;
  
  /* total number of losses by this ranked object */
  uint32_t numLosses;
  
  /* total number of draws by this ranked object */
  uint32_t numDraws;

  /* total number of ties by this ranked object */
  uint32_t numTies;

  /* total number of plies played by this ranked object */
  uint32_t numPlies;

  /* has state been written to disk? */
  bool stateSaved;

public:
  static const uint64_t defaultHash;
  static const std::string header;

  virtual ~Ranked() { };
  Ranked(size_t generation = 0, const trueSkillSettings& settings = trueSkillSettings::defaultSettings);

  virtual const std::string& getName() const;

  TrueSkill& getSkill()
  {
    return skill;
  };

  const TrueSkill& getSkill() const
  {
    return skill;
  };

  uint32_t getNumWins() const
  {
    return numWins;
  };

  uint32_t getNumLosses() const
  {
    return numLosses;
  };

  uint32_t getNumDraws() const
  {
    return numDraws;
  };

  uint32_t getNumTies() const
  {
    return numTies;
  };

  virtual uint32_t getNumPlies() const
  {
    return numPlies;
  };

  size_t getGeneration() const
  {
    return generation;
  };

  uint32_t getNumGamesPlayed() const
  {
    return numWins + numTies + numDraws + numLosses;
  };

  fpType getAveragePliesPerGame() const
  {
    size_t numPlayed = getNumGamesPlayed();
    if (numPlayed == 0) { return 0.0; }
    return  ((fpType)getNumPlies() / (fpType)getNumGamesPlayed());
  };

  bool operator<(const Ranked& other) const
  {
    return getSkill() > other.getSkill();
  };

  virtual uint64_t getHash() const { return defaultHash; };

  bool compareHash(uint64_t oHash) const
  {
    return getHash() == oHash;
  };

  bool operator==(const Ranked& other) const
  {
    return getHash() == other.getHash();
  };

  bool operator!=(const Ranked& other) const
  {
    return !(*this == other);
  };

  size_t update(const Game& cGame, const TrueSkillTeam& cTeam, size_t iTeam);

  virtual void resetRecord()
  {
    numWins = 0;
    numTies = 0;
    numDraws = 0;
    numLosses = 0;
    numPlies = 0;
  };

  bool isStateSaved() const
  {
    return stateSaved;
  };

  void setStateSaved()
  {
    stateSaved = true;
  };

  /* output the ranked preamble to this object */
  virtual void output(std::ostream& oFile, bool printHeader = true) const;

  /* input the ranked preamble to this object */
  virtual bool input(const std::vector<std::string>& lines, size_t& firstLine);

  friend std::ostream& operator <<(std::ostream& os, const Ranked& r);
};

std::ostream& operator <<(std::ostream& os, const Ranked& tR);

#endif /* RANKED_H */
