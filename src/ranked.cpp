
#include "../inc/ranked.h"

#include <boost/foreach.hpp>

#include "../inc/true_skill.h"
#include "../inc/game.h"
#include "../inc/init_toolbox.h"

const std::string Ranked::header = "PKART0";
const uint64_t Ranked::defaultHash = UINT64_MAX;

Ranked::Ranked(size_t _generation, const trueSkillSettings& settings)
  : skill(settings),
  generation(_generation),
  numWins(0),
  numLosses(0),
  numDraws(0),
  numTies(0),
  numPlies(0),
  stateSaved(false)
{
};

const std::string& Ranked::getName() const
{
  static const std::string unnamed("-UNNAMED RANKED OBJ-");
  return unnamed;
};

size_t Ranked::update(const Game& cGame, const TrueSkillTeam& cTeam, size_t iTeam)
{
  BOOST_FOREACH(const GameResult& cGameResult, cGame.getGameResults())
  {
    // update plies
    numPlies += cGameResult.numPlies;

    // update wins, losses, etc
    switch(cGameResult.endStatus)
    {
    case MATCH_DRAW:
      numDraws++;
      break;
    case MATCH_TIE:
      numTies++;
      break;
    case MATCH_TEAM_A_WINS:
      if (iTeam==TEAM_A) { numWins++; }
      else { numLosses++; }
      break;
    case MATCH_TEAM_B_WINS:
      if (iTeam==TEAM_A) { numLosses++; }
      else { numWins++; }
      break;
    }
  } // endOf foreach game in heat

  stateSaved = false;
  assert(getAveragePliesPerGame() <= (MAXPLIES + 1.0));
  return cGame.getGameResults().size();
} // endOf update

std::ostream& operator <<(std::ostream& os, const Ranked& tR)
{
  size_t prevPrecision = os.precision();
  os.precision(6);
  os <<
    " g= " << std::setw(3) << std::right << tR.getGeneration() <<
    " m= " << std::setw(7) << tR.skill().getMean() <<
    " s= " << std::setw(7) << tR.skill().getStdDev() <<
    " w= " << std::setw(7) << std::left << tR.getNumWins() << 
    " / " << std::setw(7) << std::right << (tR.getNumGamesPlayed());
  os.precision(prevPrecision);
  
  return os;
}

void Ranked::output(std::ostream& oFile, bool printHeader) const
{
  // header:
  if (printHeader)
  {
    oFile << header << "\t";
  };

  // generation:
  oFile << getGeneration() << "\t";

  oFile << getNumPlies() << "\t";

  // wins, losses, ties, draws:
  oFile 
    << getNumWins() <<
    "\t" << getNumLosses() <<
    "\t" << getNumTies() <<
    "\t" << getNumDraws() <<
    "\n";

  // trueSkill:
  skill.output(oFile);
};

bool Ranked::input(const std::vector<std::string>& lines, size_t& iLine)
{
  // are the enough lines in the input stream:
  if ((lines.size() - iLine) < 1U)
  {
    std::cerr << "ERR " << __FILE__ << "." << __LINE__ << 
      ": unexpected end of input stream at line " << iLine << "!\n";
    return false; 
  }

  // compare trueskill header:
  if (lines.at(iLine).compare(0, header.size(), header) != 0)
  {
    std::cerr << "ERR " << __FILE__ << "." << __LINE__ << 
      ": trueSkill stream has header of type \"" << lines.at(0).substr(0, header.size()) << 
      "\" (needs to be \"" << header <<
      "\") and is incompatible with this program!\n";

    return false;
  }
  // input ranked:
  {
    std::vector<std::string> tokens = INI::tokenize(lines.at(iLine), "\t");
    if (!INI::checkRangeB(tokens.size(), (size_t)7, (size_t)7)) { return false; }

    if (!INI::setArgAndPrintError("rank generation", tokens.at(1), generation, iLine, 1)) { return false; }
    if (!INI::setArgAndPrintError("rank numPlies", tokens.at(2), numPlies, iLine, 2)) { return false; }
    if (!INI::setArgAndPrintError("rank numWins", tokens.at(3), numWins, iLine, 3)) { return false; }
    if (!INI::setArgAndPrintError("rank numLosses", tokens.at(4), numLosses, iLine, 4)) { return false; }
    if (!INI::setArgAndPrintError("rank numTies", tokens.at(5), numTies, iLine, 5)) { return false; }
    if (!INI::setArgAndPrintError("rank numDraws", tokens.at(6), numDraws, iLine, 6)) { return false; }
  }
  // since we just loaded this team from memory, state is saved
  stateSaved = true;
  iLine++;

  if (!skill.input(lines, iLine)) { return false; } 

  return true;
};
