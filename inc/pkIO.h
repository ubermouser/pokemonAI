
#ifndef _PKAI_IO_H
#define	_PKAI_IO_H

#include "../inc/pkai.h"

#include <stdint.h>

// C++:
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <stdint.h>

#include <boost/tokenizer.hpp>

#include <boost/filesystem/path.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/fstream.hpp>

// C:

class PokemonAI;
class TeamNonVolatile;
class ranked_team;
class ranked_neuralNet;
class neuralNet;

// CLASS / FUNCTION LIST:
class PkIO
{
private:
  class PokemonAI* parent;
  
  PkIO() { };
public:
  std::vector<std::string> input_teams;
  std::vector<std::string> input_networks;

  PkIO(class PokemonAI* _parent);
  ~PkIO() { };

  static bool parseArg(const std::string& input, PkIO*& target);
  
  static bool inputRankedTeam(const boost::filesystem::path& path, ranked_team& cTeam); 
  static bool inputPlayerTeam(const boost::filesystem::path& path, TeamNonVolatile& cTeam);

  static bool inputRankedNetwork(const boost::filesystem::path& path, ranked_neuralNet& cNet);
  static bool inputPlayerNetwork(const boost::filesystem::path& path, neuralNet& cNet);

};

#endif	/* _PKAI_IO_H */

