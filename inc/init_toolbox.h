#ifndef INIT_TOOLBOX_H
#define INIT_TOOLBOX_H

#include "../inc/pkai.h"

#include <vector>
#include <string>
#include <sstream>
#include <iostream>
#include <iomanip>

#define BOOST_FILESYSTEM_VERSION 2
#include <boost/filesystem/path.hpp>

namespace INI
{
	std::vector<std::string> tokenize(const std::string& inputBuffer, const std::string& delimitor);

	bool loadFileToString(const boost::filesystem::path& location, const std::string& testHeader, std::string& inputBuffer);

	template <class unknownType>
	bool parseIni(const std::string& filename, unknownType*& cClass)
	{
		std::string inputBuffer;

		if (loadFileToString(filename, "", inputBuffer) != true)
		{
			return false;
		}

		std::vector<std::string> lines = tokenize(inputBuffer, "\n\r");
		for (size_t iLine = 0; iLine < lines.size(); iLine++)
		{
			std::string currentLine = lines.at(iLine);

			if (currentLine[0] == '#')
			{
				if (verbose >= 7)
				{
					std::cout << "C:\"" << currentLine << 
						"\"l=" << (iLine + 1) << 
						"\n";
				}
			}
			else if (currentLine[0] == 0)
			{
				// skip empty line
			}
			else
			{
				if (verbose >= 7)
				{
					std::cout << "P:\"" << currentLine << 
						"\"l=" << (iLine + 1) << 
						"\n";
				}
				if (!cClass->parseArg(currentLine, cClass))
				{
					std::cerr << "ERR " << __FILE__ << "." << __LINE__ << 
						": Syntax or parse error in file \"" << filename << 
						"\", line " << (iLine + 1) << 
						".\n";
					return false;
				}
			}
		}

		return true;
	} // end of parseIni

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
	void printSetArg(const std::string& commandName, const unknownType& value)
	{
		if (verbose >= 6)
		{
			std::clog << "\tset \"" << commandName << "\"=\"" << value << "\"\n";
		}
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

	template <class unknownType>
	bool checkRangeB(const unknownType& value, const unknownType& min, const unknownType& max)
	{
		if ((value >= min) && (value <= max)) { return true; }

		std::cerr << "ERR " << __FILE__ << "." << __LINE__ << 
			": " << typeid(value).name() << "=" << value << "; Range " << min << ".." << max << " \n";

		return false;
	};
};

#endif /* INIT_TOOLBOX_H */
