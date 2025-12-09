#include "pokemonai/init_toolbox.h"

#include <boost/filesystem/path.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/fstream.hpp>

#include <boost/tokenizer.hpp>

void INI::incorrectArgs( const std::string& commandName, size_t line, size_t value)
{
  std::cerr << "ERR " << __FILE__ << "." << __LINE__ << 
    ": Parse error for args of argument \"" << commandName << "\"";

  if (line != SIZE_MAX ) { std::cerr << ", line #" << line; }
  if (value != SIZE_MAX) { std::cerr << " value #" << value; }
  std::cerr << "\n";
};

void INI::incorrectNumArgs( const std::string& commandName, size_t found, size_t needed)
{
  std::cerr << "ERR " << __FILE__ << "." << __LINE__ << ": Insufficient args for argument \"" << commandName << "\". Found " << found << ", Needed " << needed << ".\n";
};

void INI::notInitialized(const std::string& commandName)
{
  std::cerr << "ERR " << __FILE__ << "." << __LINE__ << ": variable \"" << commandName << "\" must be set before this invocation is called!\n";
};





bool INI::setArg(const std::string& _token, bool& location)
{
  if (_token.compare("true") == 0)
  {
    location = true;
    return true;
  }
  else if (_token.compare("false") == 0)
  {
    location = false;
    return true;
  }
  else
  {
    std::istringstream tokenStream(_token, std::istringstream::in);

    if (!(tokenStream >> location)) { return false; }

    return true;
  }
};





std::vector<std::string> INI::tokenize(const std::string& inputBuffer, const std::string& delimitor)
{
  std::vector<std::string> result;

  boost::char_separator<char> separator(delimitor.c_str());
  boost::tokenizer<boost::char_separator<char> > _tokenizer(inputBuffer, separator);

  for (boost::tokenizer<boost::char_separator<char> >::iterator cToken = _tokenizer.begin(); cToken != _tokenizer.end(); ++cToken)
  {
    result.push_back(*cToken);
  }

  return result;
} // end of tokenize





bool INI::loadFileToString(const boost::filesystem::path& location, const std::string& testHeader, std::string& inputBuffer)
{
  /*
  * Header data:
  * <HEADER> <DATA.1> <DATA.2> ... <DATA.N-1> <DATA.N>\n
  * <NAME.1> <PROPERTY.1.1> <PROPERTY.1.2> ... <PROPERTY.1.N-1> <PROPERTY.1.N>
  * <NAME.2> <PROPERTY.2.1> <PROPERTY.2.2> ... <PROPERTY.2.N-1> <PROPERTY.2.N>
  */

  // determine if folder exists:
  if (!boost::filesystem::exists(location))
  {
    std::cerr << "ERR " << __FILE__ << "." << __LINE__ << 
      ": path \"" << location << 
      "\" must exist!\n";
    return false;
  }

  // determine if folder is a directory:
  if (boost::filesystem::is_directory(location))
  {
    std::cerr << "ERR " << __FILE__ << "." << __LINE__ << 
      ": path \"" << location << "\" is a directory!\n";
    return false;
  }

  //open file for reading
  boost::filesystem::ifstream file(location, std::ios::in | std::ios::binary); // read in, binary
  if (!file.is_open())
  {
    std::cerr << "ERR " << __FILE__ << "." << __LINE__ << 
      ": Unable to open file for binary reading at \"" << location << "\"!\n";
    return false;
  }

  // place entire file into memory here:
  inputBuffer = std::string((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
  uint64_t freadCount = inputBuffer.size();

  // no need for file now, close it
  file.close();

  if (verbose >= 6)
  {
    std::cout << "INF " << __FILE__ << "." << __LINE__ << 
      ": read " << freadCount << " bytes from file \"" << location << "\".\n";
  }
  
  // do not check for a header if none is defined
  if (testHeader.empty()) { return true; }

  // test header data for correctness
  if (inputBuffer.compare(0, testHeader.length(), testHeader, 0, testHeader.length()) != 0)
  {
    std::cerr << "ERR " << __FILE__ << "." << __LINE__ << 
      ": File \"" << location << "\" has header of type \"" << inputBuffer.substr(0, testHeader.length()) << 
      "\" (needs to be \"" << testHeader << "\") and is incompatible with this program!\n";

    return false;
  }

  return true;
} //endOf loadFileToString
