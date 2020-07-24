#include "../inc/pokemonAI.h"

#include <iostream>

#include "../inc/pkIO.h"
#include "../inc/init_toolbox.h"





int main(int argc, char** argv)
{
  class pokemonAI PokemonAI;

  int indexArg;
  int loadSuccess;
  srand((unsigned long) SRANDOMSEED);
  if (verbose >= 0) std::cout << "\"pokemonAI-" << PRECISION << "\" Independent project by David J Rendleman of the University of Pittsburgh of Computer Science and Cognitive Psychology for Summer 2010 - Fall 2012.\nContact at David.Rendleman+PKAI@gmail.com\n\n";
  loadSuccess = INI::parseIni("pkai.ini", PokemonAI.getIO());

  //parse command line args next:
  for (indexArg = 1; indexArg < argc; indexArg++)
  {
    if (pkIO::parseArg(argv[indexArg], PokemonAI.getIO()) == false)
    {
      std::cerr << "ERR " << __FILE__ << "." << __LINE__ << 
          ": Unrecognized argument in command line \"" << argv[indexArg] << "\"! See readme and pkai.ini file for information about the operation of this program.\n";
      exit(EXIT_FAILURE);
    }
  }

  if (!((loadSuccess) || (argc > 1))) { if (verbose >= 0) std::cout << "Nothing to do, exiting.\n"; return(EXIT_SUCCESS); }

  if (verbose >= 0) std::cout << "Loading Pokemon game...\n";
  if (PokemonAI.init() == false) { return(EXIT_FAILURE); }

  if (PokemonAI.run() == false) { return(EXIT_FAILURE); }
  
  // operation successful!
  return (EXIT_SUCCESS);
}
