
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

class pokemonAI;
class team_nonvolatile;
class ranked_team;
class ranked_neuralNet;
class neuralNet;

// CLASS / FUNCTION LIST:
class pkIO
{
private:
  class pokemonAI* parent;
  
  pkIO() { };
public:
  std::string input_moveLibrary;
  std::string input_pokemonLibrary;
  std::string input_natureLibrary;
  std::string input_itemLibrary;
  std::string input_abilityLibrary;
  std::string input_typeLibrary;
  std::string input_movelistLibrary;
  std::string input_metagameLibrary;
  std::string input_pluginFolder;
  std::vector<std::string> input_teams;
  std::vector<std::string> input_networks;

  pkIO(class pokemonAI* _parent);
  ~pkIO() { };

  static bool parseArg(const std::string& input, pkIO*& target);


  bool inputMoves(); // the moves themselves
  bool inputPokemon(); // pokemon species
  bool inputAbilities(); // abilities pokemon might have
  bool inputNatures(); // natures pokemon might have
  bool inputItems(); // items pokemon might carry
  bool inputTypes(); // types pokemon might have
  bool inputMovelist(); // the moves that each pokemon has
  
  static bool inputRankedTeam(const boost::filesystem::path& path, ranked_team& cTeam); 
  static bool inputPlayerTeam(const boost::filesystem::path& path, team_nonvolatile& cTeam);

  static bool inputRankedNetwork(const boost::filesystem::path& path, ranked_neuralNet& cNet);
  static bool inputPlayerNetwork(const boost::filesystem::path& path, neuralNet& cNet);

  bool inputPlugins(); // input scripts for registered moves

};

#endif	/* _PKAI_IO_H */

