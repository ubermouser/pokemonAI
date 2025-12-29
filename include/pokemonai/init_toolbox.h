#ifndef INIT_TOOLBOX_H
#define INIT_TOOLBOX_H

#include "pokemonai/pkai.h"

#include <vector>
#include <string>
#include <sstream>
#include <iostream>
#include <iomanip>

#include <boost/filesystem/path.hpp>

namespace INI
{
  std::vector<std::string> tokenize(const std::string& inputBuffer, const std::string& delimitor);

  bool loadFileToString(const boost::filesystem::path& location, const std::string& testHeader, std::string& inputBuffer);

  void incorrectNumArgs( const std::string& commandName, size_t found, size_t needed);

  void notInitialized(const std::string& commandName);

  void incorrectArgs( const std::string& commandName, size_t line = SIZE_MAX, size_t value = SIZE_MAX);

  bool setArg(const std::string& _token, bool& location);

  template <class unknownType> 
  bool setArg(const std::string& _token, unknownType& location)
  {
    std::istringstream tokenStream(_token, std::istringstream::in);

    if (!(tokenStream >> location)) { return false; }

    return true;
  };

  template <class unknownType>
  bool setArgAndPrintError(
    const std::string& name, 
    const std::string& token, 
    unknownType& location,
    size_t iLine = SIZE_MAX,
    size_t iToken = SIZE_MAX)
  {
    if (!setArg(token, location))
    {
      incorrectArgs(name, iLine, iToken);
      return false;
    }
    return true;
  };

  template <class unknownType, class valueType>
  bool checkRangeB(const unknownType& value, const valueType& min, const valueType& max)
  {
    if ((value >= unknownType(min)) && (value <= unknownType(max))) { return true; }

    SPDLOG_CRITICAL(
        "{} out of range: {} not in {}..{}.",
        typeid(value).name(),
        value,
        min,
        max);

    return false;
  };
};

#endif /* INIT_TOOLBOX_H */
