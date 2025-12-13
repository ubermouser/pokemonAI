#include <string>
#include <iostream>
#include <iomanip>
#include <vector>
#include <assert.h>
#include <boost/array.hpp>

#include <boost/foreach.hpp>

#define BOOST_FILESYSTEM_VERSION 2
#include <boost/filesystem/path.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/fstream.hpp>

#include "pokemonai/ranked.h"

#include "pokemonai/init_toolbox.h"

void printRankedInfo(const boost::filesystem::path& cPath)
{
  ranked cRanked;
  {
    std::vector<std::string> lines; 
    size_t iLine = 0; 
    { 
      std::string inputBuffer; 
      INI::loadFileToString(cPath, "", inputBuffer); 
      lines = INI::tokenize(inputBuffer, "\n\r"); 
    } 
      
    // find ranked object header within file: (it's generally near the front)
    for ( ; iLine != lines.size(); ++iLine)
    {
      if (lines[iLine].compare(0, ranked::header.size(), ranked::header) == 0) { break; }
    }

    if (iLine == lines.size()) { return; } // file contains no ranked object
      
    // load ranked object:
    if (!cRanked.input(lines, iLine)) { return; } // failed loading ranked object
  }

  // print descriptives of ranked object:
  std::cout <<
    "n= " << std::setw(22) << cPath.stem() <<
    " m= " << std::setw(7) << cRanked.getSkill().getMean() <<
    " s= " << std::setw(7) << cRanked.getSkill().getStdDev() <<
    " g= " << std::setw(3) << cRanked.getGeneration() <<
    " w= " << std::setw(7) << cRanked.getNumWins() <<
    " l= " << std::setw(7) << cRanked.getNumLosses() <<
    " t= " << std::setw(7) << cRanked.getNumTies() <<
    " d= " << std::setw(7) << cRanked.getNumDraws() <<
    "\n";
  std::cout.flush();
}

int main(int argv, char** argc)
{
  std::vector<std::string> args(argc, argc+argv);
  verbose = 5;
  std::cout.precision(6);

  if (args.size() < 2)
  {
    std::cerr 
      << "Usage: printRankedObjects [OPTION]... [<directory>|<file>]"
      << "\n\tprints detailed characteristics of all ranked objects within a given directory"
      //<< "\n\t-desriptive\tprints additional descriptive statistics"
      << "\n";
    return EXIT_FAILURE;
  }
  for (size_t iArg = 1; iArg != args.size(); ++iArg)
  {
    boost::filesystem::path directory(args.at(iArg));

    // determine if folder exists:
    if (!boost::filesystem::exists(directory))
    {
      std::cerr << "INF " << __FILE__ << "." << __LINE__ << 
        ": No directory or file found at location \"" << directory << 
        "\"!\n";
      return EXIT_FAILURE;
    }

    // determine if folder is a directory:
    if (!boost::filesystem::is_directory(directory))
    {
      if (!is_regular_file(directory))
      {
        std::cerr << "ERR " << __FILE__ << "." << __LINE__ << 
          ": \"" << directory << "\" is not a directory or normal file!\n";
        return EXIT_FAILURE;
      }

      std::cerr << "output of file \"" << directory << "\"...\n";
      std::cerr.flush();
      std::cout.flush();

      printRankedInfo(directory);
    }
    else
    {
      std::cerr << "output of directory \"" << directory << "\"...\n";
      std::cerr.flush();
      std::cout.flush();

      boost::filesystem::directory_iterator it(directory), eod;
      BOOST_FOREACH(boost::filesystem::path const &cPath, std::make_pair(it, eod))   
      {
        // ignore directories
        if( !is_regular_file(cPath)) { continue; } 

        // make sure extension is txt, ignore all others:
        if ( cPath.extension().compare(".txt") != 0) { continue; }

        printRankedInfo(cPath);

      } // endOf foreach file
    }
  }

  return EXIT_SUCCESS;
}
