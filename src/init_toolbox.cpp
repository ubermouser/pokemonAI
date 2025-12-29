#include "pokemonai/init_toolbox.h"

#include <boost/filesystem/path.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/fstream.hpp>

#include <boost/tokenizer.hpp>

void INI::incorrectArgs( const std::string& commandName, size_t line, size_t value)
{
  if (line != SIZE_MAX && value != SIZE_MAX) {
    SPDLOG_ERROR(
        "Parse error for args of argument \"{}\", line #{} value #{}",
        commandName,
        line,
        value);
  } else if (line != SIZE_MAX) {
    SPDLOG_ERROR(
        "Parse error for args of argument \"{}\", line #{}", commandName, line);
  } else if (value != SIZE_MAX) {
    SPDLOG_ERROR(
        "Parse error for args of argument \"{}\", value #{}",
        commandName,
        value);
  } else {
    SPDLOG_ERROR("Parse error for args of argument \"{}\"", commandName);
  }
};

void INI::incorrectNumArgs( const std::string& commandName, size_t found, size_t needed)
{
  SPDLOG_ERROR(
      "Insufficient args for argument \"{}\". Found {}, Needed {}.",
      commandName,
      found,
      needed);
};

void INI::notInitialized(const std::string& commandName)
{
  SPDLOG_ERROR(
      "variable \"{}\" must be set before this invocation is called!",
      commandName);
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
    SPDLOG_ERROR("path \"{}\" must exist!", location.string());
    return false;
  }

  // determine if folder is a directory:
  if (boost::filesystem::is_directory(location))
  {
    SPDLOG_ERROR("path \"{}\" is a directory!", location.string());
    return false;
  }

  //open file for reading
  boost::filesystem::ifstream file(location, std::ios::in | std::ios::binary); // read in, binary
  if (!file.is_open())
  {
    SPDLOG_ERROR(
        "Unable to open file for binary reading at \"{}\"!", location.string());
    return false;
  }

  // place entire file into memory here:
  inputBuffer = std::string((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
  uint64_t freadCount = inputBuffer.size();

  // no need for file now, close it
  file.close();

  SPDLOG_TRACE(
      "read {} bytes from file \"{}\".", freadCount, location.string());

  // do not check for a header if none is defined
  if (testHeader.empty()) { return true; }

  // test header data for correctness
  if (inputBuffer.compare(0, testHeader.length(), testHeader, 0, testHeader.length()) != 0)
  {
    SPDLOG_ERROR(
        "file \"{}\" has header of type \"{}\" (needs to be \"{}\") and is "
        "incompatible with this program!",
        location.string(),
        inputBuffer.substr(0, testHeader.length()),
        testHeader);

    return false;
  }

  return true;
} //endOf loadFileToString
