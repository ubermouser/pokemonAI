
//#define PKAI_EXPORT
#include "../inc/pokedex.h"

#include <boost/extension/shared_library.hpp>
#include <boost/function.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/fstream.hpp>

#include "../inc/init_toolbox.h"
#include "../inc/orphan.h"

#include "../inc/plugin.h"
#include "../inc/pluggable.h"

using namespace boost::extensions;

using namespace INI;
using namespace orphan;

int verbose = 0;
int warning = 0;
const pokedex* pkdex = NULL;


bool pokedex::inputMovelist(const std::vector<std::string>& lines, size_t& iLine)
{
	static const std::string header = "PKAIZ";
	/*
	 * Header data:
	 * PKAIZ <SIZE> #fluff\n
	 * #fluff line\n
	 * <String Pokemon>\t<String Movename>
	 */

	// are the enough lines in the input stream for at least the header:
	if ((lines.size() - iLine) < 2U)
	{
		std::cerr << "ERR " << __FILE__ << "." << __LINE__ << 
			": unexpected end of input stream at line " << iLine << "!\n";
		return false; 
	}

	// compare header:
	if (lines.at(iLine).compare(0, header.size(), header) != 0)
	{
		std::cerr << "ERR " << __FILE__ << "." << __LINE__ << 
			": movelist inputStream has header of type \"" << lines.at(iLine).substr(0, header.size()) << 
			"\" (needs to be \"" << header <<
			"\") and is incompatible with this program!\n";

		return false;
	}

	// guess size of array:
	size_t _numElements;
	{
		std::vector<std::string> tokens = INI::tokenize(lines.at(iLine), "\t");
		if (!INI::checkRangeB(tokens.size(), (size_t)2, tokens.max_size())) { return false; }

		if (!INI::setArgAndPrintError("movelist numElements", tokens.at(1), _numElements, iLine, 1)) { return false; }

		if (!INI::checkRangeB(_numElements, (size_t)0, (size_t)MAXMOVELIST)) { return false; }
	}

	// ignore fluff line
	iLine+=2;

	if (!INI::checkRangeB(lines.size() - iLine, (size_t)_numElements, (size_t)MAXMOVELIST)) { return false; }

	// check if pokemon has been initialized yet
	if (getPokemon().empty() || getMoves().empty())
	{
		std::cerr << "ERR " << __FILE__ << "." << __LINE__ <<
			": Pokemon and moves must be initialized before adding movelists to them!\n";
		return false;
	}

	std::vector<std::string> mismatchedPokemon;
	std::vector<std::string> mismatchedMoves;

	std::vector<pokemon_base>::iterator cPokemon = getPokemon().begin();
	std::vector<move>::iterator cMove = getMoves().begin();
	
	for (size_t iPair = 0; iPair < lines.size() - 2; ++iPair, ++iLine)
	{
		std::vector<std::string> tokens = tokenize(lines.at(iLine), "\t");
		if (tokens.size() != 2)
		{
			std::cerr << "ERR " << __FILE__ << "." << __LINE__ << 
				": movelist inputStream has malformed line #" << (iLine) << 
				" with " << tokens.size() << " values!\n";
			return false;
		}

		//find pokemon referred to by this pair
		//TODO: make sure this is actually faster than binary search
		//WARNING: THIS CODE ASSUMES: NO DUPLICATES, ALPHABETICAL ORDER OF POKEMON
		while(cPokemon->getName().compare(tokens.at(0)) != 0)
		{
			cPokemon++;
			if (cPokemon != getPokemon().end())
			{
				cMove = getMoves().begin(); // reset moves
			}
			else // unable to find target pokemon in move pair:
			{
				// add orphan:
				orphanAddToVector(mismatchedPokemon, tokens.at(0));
				
				// reset pokemon and moves:
				cPokemon = getPokemon().begin();
				cMove = getMoves().begin();

				goto outerWhile;
			}
		}

		//find move referred to by this pair
		//TODO: make sure this is actually faster than binary search
		//WARNING: THIS CODE ASSUMES: NO DUPLICATES, ALPHABETICAL ORDER OF MOVES
		while(cMove->getName().compare(tokens.at(1)) != 0)
		{
			cMove++;
			if (cMove == getMoves().end())
			{
				// add orphan:
				orphanAddToVector(mismatchedMoves, tokens.at(1));

				// reset moves
				cMove = getMoves().begin();

				goto outerWhile;
			}
		}
		// add this name to the current Pokemon's move list.
		cPokemon->movelist.push_back(cMove - getMoves().begin());

	outerWhile:
		std::cerr.flush(); // dummy function
	} //end of per-pair

	// mark orphans with no movesets, sort the movesets of bases which have them:
	for (size_t iPokemon = 0; iPokemon != getPokemon().size(); ++iPokemon)
	{
		pokemon_base& cBase = getPokemon()[iPokemon];
		if (cBase.movelist.empty()) { cBase.lostChild = true; continue;}
		std::sort(cBase.movelist.begin(), cBase.movelist.end());
	}

	//output orphans
	if (mismatchedPokemon.size() != 0)
	{
		if (verbose >= 5) std::cerr << "WAR " << __FILE__ << "." << __LINE__ << 
			": movelist inputStream - " << mismatchedPokemon.size() << " Orphaned movelist-pokemon!\n";
		if (verbose >= 6)
		{
			for (size_t indexOrphan = 0; indexOrphan < mismatchedPokemon.size(); indexOrphan++)
			{
				std::cout << "\tOrphaned pokemon \"" << mismatchedPokemon.at(indexOrphan) << "\"\n";
			}
		}
	}
	if (mismatchedMoves.size() != 0)
	{
		if (verbose >= 5) std::cerr << "WAR " << __FILE__ << "." << __LINE__ << 
			": movelist inputStream - " << mismatchedMoves.size() << " Orphaned movelist-moves!\n";
		if (verbose >= 6)
		{
			for (size_t indexOrphan = 0; indexOrphan < mismatchedMoves.size(); indexOrphan++)
			{
				std::cout << "\tOrphaned move \"" << mismatchedMoves.at(indexOrphan) << "\"\n";
			}
		}
	}

	return true; // import success
}//endof import movelist





bool pokedex::inputPokemon(const std::vector<std::string>& lines, size_t& iLine)
{
	static const std::string header = "PKAIS";
	/*
	 * Header data:
	 * PKAIS <SIZE> #fluff\n
	 * #fluff line\n
	 * <String NAME>\t<float 1>\t<float 2>\t<float ... SIZE>
	 */

	// are the enough lines in the input stream for at least the header:
	if ((lines.size() - iLine) < 2U)
	{
		std::cerr << "ERR " << __FILE__ << "." << __LINE__ << 
			": unexpected end of input stream at line " << iLine << "!\n";
		return false; 
	}

	// compare header:
	if (lines.at(iLine).compare(0, header.size(), header) != 0)
	{
		std::cerr << "ERR " << __FILE__ << "." << __LINE__ << 
			": pokemon inputStream has header of type \"" << lines.at(iLine).substr(0, header.size()) << 
			"\" (needs to be \"" << header <<
			"\") and is incompatible with this program!\n";

		return false;
	}

	// guess size of array:
	size_t _numElements;
	{
		std::vector<std::string> tokens = INI::tokenize(lines.at(iLine), "\t");
		if (!INI::checkRangeB(tokens.size(), (size_t)2, tokens.max_size())) { return false; }

		if (!INI::setArgAndPrintError("pokemon numElements", tokens.at(1), _numElements, iLine, 1)) { return false; }

		if (!INI::checkRangeB(_numElements, (size_t)0, (size_t)MAXPOKEMON)) { return false; }
	}

	// ignore fluff line
	iLine+=2;

	// make sure number of lines in the file is correct for the number of moves we were given:
	if (!INI::checkRangeB(lines.size() - iLine, (size_t)_numElements, (size_t)MAXPOKEMON)) { return false; }
	getPokemon().resize(_numElements);

	std::vector<std::string> mismatchedTypes;
	std::vector<std::string> mismatchedAbilities;

	for (size_t iPokemon = 0; iPokemon < getPokemon().size(); ++iPokemon, ++iLine)
	{
		std::vector<std::string> tokens = tokenize(lines.at(iLine), "\t");
		class pokemon_base& cPokemon = getPokemon()[iPokemon];
		if (tokens.size() != 12)
		{
			std::cerr << "ERR " << __FILE__ << "." << __LINE__ << 
				": pokemon inputStream has malformed line #" << (iLine) << 
				" with " << tokens.size() << " values!\n";
			return false;
		}
		// init lostChild
		cPokemon.lostChild = false;

		//pokemon name
		size_t iToken = 0;
		cPokemon.setName(tokens.at(iToken));

		//pokemon primary type
		iToken = 1;
		{
			size_t type = orphanCheck(getTypes(), &mismatchedTypes, tokens.at(iToken));
			if (type == SIZE_MAX) { cPokemon.lostChild = true; } //orphan!
			cPokemon.types[0] = &getTypes()[type];
		}

		//pokemon secondary type
		iToken = 2;
		{
			size_t type = orphanCheck(getTypes(), &mismatchedTypes, tokens.at(iToken));
			if (type == SIZE_MAX) { cPokemon.lostChild = true; } //orphan!
			cPokemon.types[1] = &getTypes()[type]; // notype = "None"
		}

		//base stats table
		iToken = 3;
		for (size_t iStat = 0; iStat < 6; iStat++)
		{
			uint32_t cStat;
			if (!setArg(tokens.at(iToken + iStat), cStat)) { incorrectArgs("cStat", iLine, iToken + iStat); return false; }
			checkRangeB(cStat, (uint32_t)1, (uint32_t)255);
			cPokemon.baseStats[iStat] = cStat;
		}

		//weight
		iToken = 9;
		{
			fpType cWeight;
			if (!setArg(tokens.at(iToken), cWeight)) { incorrectArgs("cWeight", iLine, iToken); return false; }
			checkRangeB(cWeight, (fpType)0.1, (fpType)1000.0);
			cPokemon.weight = cWeight * WEIGHTMULTIPLIER;
		}

		//first ability choice
		iToken = 10;
		{
			const ability* cAbility = orphanCheck_ptr(getAbilities(), &mismatchedAbilities, tokens.at(iToken));
			if (cAbility == NULL) { cPokemon.lostChild = true; } //orphan!
			else { cPokemon.abilities.push_back(cAbility); }
		}

		//second ability choice
		iToken = 11;
		if (tokens.at(iToken).compare("---") != 0)
		{
			//strncpy(currentPokemon->secondaryAbilityName,tokens.at(11),20);
			const ability* cAbility = orphanCheck_ptr(getAbilities(), &mismatchedAbilities, tokens.at(iToken));
			if (cAbility == NULL) { cPokemon.lostChild = true; } //orphan!
			else { cPokemon.abilities.push_back(cAbility); }
		}

		// sort abilities (by pointer):
		{
			std::sort(cPokemon.abilities.begin(), cPokemon.abilities.end());
		}

	} //end of per-pokemon

	//output orphans
	if (mismatchedTypes.size() != 0)
	{
		if (verbose >= 5) std::cerr << "WAR " << __FILE__ << "." << __LINE__ << 
			": pokemon inputStream - " << mismatchedTypes.size() << " Orphaned pokemon-types!\n";
		if (verbose >= 6)
		{
			for (size_t indexOrphan = 0; indexOrphan < mismatchedTypes.size(); indexOrphan++)
			{
				std::cout << "\tOrphaned type \"" << mismatchedTypes.at(indexOrphan) << "\"\n";
			}
		}
	}
	if (mismatchedAbilities.size() != 0)
	{
		if (verbose >= 5) std::cerr << "WAR " << __FILE__ << "." << __LINE__ << 
			": pokemon inputStream  - " << mismatchedAbilities.size() << " Orphaned pokemon-abilities!\n";
		if (verbose >= 6)
		{
			for (size_t indexOrphan = 0; indexOrphan < mismatchedAbilities.size(); indexOrphan++)
			{
				std::cout << "\tOrphaned ability \"" << mismatchedAbilities.at(indexOrphan) << "\"\n";
			}
		}
	}

	return true; // import success
} // endof import pokemon





bool pokedex::inputAbilities(const std::vector<std::string>& lines, size_t& iLine)
{
	static const std::string header = "PKAIA";
	/*
	 * Header data:
	 * PKAIA <SIZE> #fluff\n
	 * #fluff line\n
	 * <String NAME>\t<String SCRIPT>
	 */

	// are the enough lines in the input stream for at least the header:
	if ((lines.size() - iLine) < 2U)
	{
		std::cerr << "ERR " << __FILE__ << "." << __LINE__ << 
			": unexpected end of input stream at line " << iLine << "!\n";
		return false; 
	}

	// compare header:
	if (lines.at(iLine).compare(0, header.size(), header) != 0)
	{
		std::cerr << "ERR " << __FILE__ << "." << __LINE__ << 
			": ability inputStream has header of type \"" << lines.at(iLine).substr(0, header.size()) << 
			"\" (needs to be \"" << header <<
			"\") and is incompatible with this program!\n";

		return false;
	}

	// guess size of array:
	size_t _numElements;
	{
		std::vector<std::string> tokens = INI::tokenize(lines.at(iLine), "\t");
		if (!INI::checkRangeB(tokens.size(), (size_t)2, tokens.max_size())) { return false; }

		if (!INI::setArgAndPrintError("abilities numElements", tokens.at(1), _numElements, iLine, 1)) { return false; }

		if (!INI::checkRangeB(_numElements, (size_t)0, (size_t)MAXABILITIES)) { return false; }
	}

	// ignore fluff line
	iLine+=2;

	// make sure number of lines in the file is correct for the number of moves we were given:
	if (!INI::checkRangeB(lines.size() - iLine, (size_t)_numElements, (size_t)MAXABILITIES)) { return false; }
	getAbilities().resize(_numElements);

	for (size_t iAbility = 0; iAbility < getAbilities().size(); ++iAbility, ++iLine)
	{
		std::vector<std::string> tokens = tokenize(lines.at(iLine), "\t");
		class ability& cAbility = getAbilities()[iAbility];
		if (tokens.size() != 2)
		{
			std::cerr << "ERR " << __FILE__ << "." << __LINE__ << 
				": ability inputStream has malformed line #" << iLine << 
				" with " << tokens.size() << " values!\n";
			return false;
		}

		//ability name
		cAbility.setName(tokens.at(0));

		//ability script
		if (tokens.at(1).compare("---") == 0)
		{ cAbility.script.clear(); }
		else
		{
			size_t tokenLength = tokens.at(1).size();
			size_t offset = 0;
			//check for quotations, and if they exist remove them
			if (tokens.at(1)[0] == '"' && tokens.at(1)[tokenLength-1] == '"')
			{
				offset = 1;
			}
			
			cAbility.script = std::string(tokens.at(1).substr(offset, tokenLength - offset));
		}

	} //end of per-ability

	return true; // import success
}





bool pokedex::inputTypes(const std::vector<std::string>& lines, size_t& iLine)
{
	static const std::string header = "PKAIT";
	/*
	 * Header data:
	 * PKAIT <SIZE> #fluff\n
	 * #fluff line\n
	 * <String NAME>\t<float 1>\t<float 2>\t<float ... SIZE>
	 */

	// are the enough lines in the input stream for at least the header:
	if ((lines.size() - iLine) < 2U)
	{
		std::cerr << "ERR " << __FILE__ << "." << __LINE__ << 
			": unexpected end of input stream at line " << iLine << "!\n";
		return false; 
	}

	// compare header:
	if (lines.at(iLine).compare(0, header.size(), header) != 0)
	{
		std::cerr << "ERR " << __FILE__ << "." << __LINE__ << 
			": types inputStream has header of type \"" << lines.at(iLine).substr(0, header.size()) << 
			"\" (needs to be \"" << header <<
			"\") and is incompatible with this program!\n";

		return false;
	}

	// guess size of array:
	size_t _numElements;
	{
		std::vector<std::string> tokens = INI::tokenize(lines.at(iLine), "\t");
		if (!INI::checkRangeB(tokens.size(), (size_t)2, tokens.max_size())) { return false; }

		if (!INI::setArgAndPrintError("types numElements", tokens.at(1), _numElements, iLine, 1)) { return false; }

		if (!INI::checkRangeB(_numElements, (size_t)0, (size_t)MAXTYPES)) { return false; }
	}

	// ignore fluff line
	iLine+=2;

	// make sure number of lines in the file is correct for the number of moves we were given:
	if (!INI::checkRangeB(lines.size() - iLine, (size_t)_numElements, (size_t)MAXTYPES)) { return false; }
	getTypes().resize(_numElements);

	for (size_t iType = 0; iType < getTypes().size(); ++iType, ++iLine)
	{
		std::vector<std::string> tokens = tokenize(lines.at(iLine), "\t");
		class type& cType = getTypes()[iType];
		if (tokens.size() != getTypes().size()+1)
		{
			std::cerr << "ERR " << __FILE__ << "." << __LINE__ << 
				": types inputStream has malformed line #" << iLine << 
				" with " << tokens.size() << " values!\n";
			return false;
		}

		//allocate dynamic modtable
		cType.modTable.resize(getTypes().size(), 0);

		//type name
		cType.setName(tokens.at(0));

		//dynamic modtable
		for (size_t indexInnerType = 0; indexInnerType < getTypes().size(); indexInnerType++)
		{
			double cTypeVal;
			if (!setArg(tokens.at(indexInnerType+1), cTypeVal)) { incorrectArgs("cTypeVal", iLine, indexInnerType+1); return false; }
			checkRangeB(cTypeVal, 0.0, 2.0);
			cType.modTable[indexInnerType] = cTypeVal * FPMULTIPLIER;
		}

	} //end of per-type

	return true; // import success
} // endof import types





bool pokedex::inputNatures(const std::vector<std::string>& lines, size_t& iLine)
{
	static const std::string header = "PKAIN";
	/*
	 * Header data:
	 * PKAIN <SIZE> #fluff\n
	 * #fluff line\n
	 * <String NAME>\t<float 1>\t<float 2>\t<float ... SIZE>
	 */

	// are the enough lines in the input stream for at least the header:
	if ((lines.size() - iLine) < 2U)
	{
		std::cerr << "ERR " << __FILE__ << "." << __LINE__ << 
			": unexpected end of input stream at line " << iLine << "!\n";
		return false; 
	}

	// compare header:
	if (lines.at(iLine).compare(0, header.size(), header) != 0)
	{
		std::cerr << "ERR " << __FILE__ << "." << __LINE__ << 
			": natures inputStream has header of type \"" << lines.at(iLine).substr(0, header.size()) << 
			"\" (needs to be \"" << header <<
			"\") and is incompatible with this program!\n";

		return false;
	}

	// guess size of array:
	size_t _numElements;
	{
		std::vector<std::string> tokens = INI::tokenize(lines.at(iLine), "\t");
		if (!INI::checkRangeB(tokens.size(), (size_t)2, tokens.max_size())) { return false; }

		if (!INI::setArgAndPrintError("natures numElements", tokens.at(1), _numElements, iLine, 1)) { return false; }

		if (!INI::checkRangeB(_numElements, (size_t)0, (size_t)MAXNATURES)) { return false; }
	}

	// ignore fluff line
	iLine+=2;

	// make sure number of lines in the file is correct for the number of moves we were given:
	if (!INI::checkRangeB(lines.size() - iLine, (size_t)_numElements, (size_t)MAXNATURES)) { return false; }
	getNatures().resize(_numElements);

	for (size_t iNature = 0; iNature < getNatures().size(); ++iNature, ++iLine)
	{
		std::vector<std::string> tokens = tokenize(lines.at(iLine), "\t");
		class nature& cNature = getNatures()[iNature];
		if (tokens.size() != 6)
		{
			std::cerr << "ERR " << __FILE__ << "." << __LINE__ << 
				": natures inputStream has malformed line #" << iLine << 
				" with " << tokens.size() << " values!\n";
			return false;
		}

		//nature name
		cNature.setName(tokens.at(0));

		//modtable
		for (size_t indexMod = 0; indexMod < 5; indexMod++)
		{
			double cNatureVal;
			if (!setArg(tokens.at(indexMod+1), cNatureVal)) { incorrectArgs("cNatureVal", iLine, indexMod+1); return false; }
			checkRangeB(cNatureVal, 0.9, 1.1);
			cNature.modTable[indexMod] = cNatureVal * FPMULTIPLIER;
		}

	} //end of per-nature

	return true; // import success
} // endof import natures





bool pokedex::inputItems(const std::vector<std::string>& lines, size_t& iLine)
{
	static const std::string header = "PKAII";
	/*
	 * Header data:
	 * PKAII <SIZE> #fluff\n
	 * #fluff line\n
	 * <String NAME>\t<String SCRIPT>
	 */

	// are the enough lines in the input stream for at least the header:
	if ((lines.size() - iLine) < 2U)
	{
		std::cerr << "ERR " << __FILE__ << "." << __LINE__ << 
			": unexpected end of input stream at line " << iLine << "!\n";
		return false; 
	}

	// compare header:
	if (lines.at(iLine).compare(0, header.size(), header) != 0)
	{
		std::cerr << "ERR " << __FILE__ << "." << __LINE__ << 
			": item inputStream has header of type \"" << lines.at(iLine).substr(0, header.size()) << 
			"\" (needs to be \"" << header <<
			"\") and is incompatible with this program!\n";

		return false;
	}

	// guess size of item array:
	size_t _numElements;
	{
		std::vector<std::string> tokens = INI::tokenize(lines.at(iLine), "\t");
		if (!INI::checkRangeB(tokens.size(), (size_t)2, tokens.max_size())) { return false; }

		if (!INI::setArgAndPrintError("items numElements", tokens.at(1), _numElements, iLine, 1)) { return false; }

		if (!INI::checkRangeB(_numElements, (size_t)0, (size_t)MAXITEMS)) { return false; }
	}

	// ignore fluff line
	iLine+=2;

	// make sure number of lines in the file is correct for the number of moves we were given:
	if (!INI::checkRangeB(lines.size() - iLine, (size_t)_numElements, (size_t)MAXITEMS)) { return false; }
	getItems().resize(_numElements);

	std::vector<std::string> mismatchedTypes;

	for (size_t iItem = 0; iItem < getItems().size(); ++iItem, ++iLine)
	{
		std::vector<std::string> tokens = tokenize(lines.at(iLine), "\t");
		class item& cItem = getItems()[iItem];
		if (tokens.size() != 8)
		{
			std::cerr << "ERR " << __FILE__ << "." << __LINE__ << 
				": item inputStream has malformed line #" << iLine << 
				" with " << tokens.size() << " values!\n";
			return false;
		}

		cItem.lostChild = false;

		//item name
		size_t iToken =  0;
		cItem.setName(tokens.at(iToken));

		// fling power:
		iToken = 1;
		if (tokens.at(iToken).compare("---") == 0)
		{ cItem.flingPower = 0; }
		else
		{ 
			uint32_t cPower;
			if (!setArg(tokens.at(iToken), cPower)) { incorrectArgs("fling power", iLine, iToken); return false; }
			checkRangeB(cPower, 1U, 255U);
			cItem.flingPower = (uint8_t) cPower;
		}

		// boosted type:
		iToken = 2;
		if (tokens.at(iToken).compare("---") == 0)
		{ cItem.boostedType = type::no_type; }
		else
		{
			const type* cType = orphanCheck_ptr(getTypes(), &mismatchedTypes, tokens.at(iToken));
			if (cType == NULL) { cItem.lostChild = true; } //orphan!
			cItem.boostedType = cType;
		}
		
		// IGNORED: pokemon affected

		// natural gift power:
		iToken = 4;
		if (tokens.at(iToken).compare("---") == 0)
		{ cItem.naturalGift_power = 0; }
		else
		{ 
			uint32_t cPower;
			if (!setArg(tokens.at(iToken), cPower)) { incorrectArgs("natural gift power", iLine, iToken); return false; }
			checkRangeB(cPower, 1U, 255U);
			cItem.naturalGift_power = (uint8_t) cPower;
		}

		// natural gift type:
		iToken = 5;
		if (tokens.at(iToken).compare("---") == 0)
		{ cItem.naturalGift_type = type::no_type; }
		else
		{
			const type* cType = orphanCheck_ptr(getTypes(), &mismatchedTypes, tokens.at(iToken));
			if (cType == NULL) { cItem.lostChild = true; } //orphan!
			cItem.naturalGift_type = cType;
		}

		// resisted type:
		iToken = 6;
		if (tokens.at(iToken).compare("---") == 0)
		{ cItem.resistedType = type::no_type; }
		else
		{
			const type* cType = orphanCheck_ptr(getTypes(), &mismatchedTypes, tokens.at(iToken));
			if (cType == NULL) { cItem.lostChild = true; } //orphan!
			cItem.resistedType = cType;
		}

		// item script
		iToken = 7;
		if (tokens.at(iToken).compare("---") == 0)
		{ cItem.setHasNoPlugins(); }
		/*else
		{
			size_t tokenLength = tokens.at(iToken).size();
			size_t offset = 0;
			//check for quotations, and if they exist remove them
			if (tokens.at(1)[0] == '"' && tokens.at(iToken)[tokenLength-1] == '"')
			{
				offset = 1;
			}
			
			cItem.script = std::string(tokens.at(iToken).substr(offset, tokenLength - offset));
		}*/
	} //end of per-item

	if (mismatchedTypes.size() != 0)
	{
		if (verbose >= 5) std::cerr << "WAR " << __FILE__ << "." << __LINE__ << 
			": item inputStream - " << mismatchedTypes.size() << " Orphaned item-types!\n";
		if (verbose >= 6)
		{
			for (size_t indexOrphan = 0; indexOrphan < mismatchedTypes.size(); indexOrphan++)
			{
				std::cout << "\tOrphaned type \"" << mismatchedTypes.at(indexOrphan) << "\"\n";
			}
		}
	}

	return true;
} // end of importItem





bool pokedex::inputMoves(const std::vector<std::string>& lines, size_t& iLine)
{
	static const std::string header = "PKAIM";
	/*
	 * Header data:
	 * PKAIM\t<SIZE>\t#fluff\n
	 * #fluff line\n
	 * <String NAME>\t<int TYPE>\t<int PP>\t<int P.Accuracy>\t<int Power>\t<int Damage>\t<int Target>\t<int Priority>\t ... etc
	 */

	// are the enough lines in the input stream for at least the header:
	if ((lines.size() - iLine) < 2U)
	{
		std::cerr << "ERR " << __FILE__ << "." << __LINE__ << 
			": unexpected end of input stream at line " << iLine << "!\n";
		return false; 
	}

	// compare header:
	if (lines.at(iLine).compare(0, header.size(), header) != 0)
	{
		std::cerr << "ERR " << __FILE__ << "." << __LINE__ << 
			": move inputStream has header of type \"" << lines.at(iLine).substr(0, header.size()) << 
			"\" (needs to be \"" << header <<
			"\") and is incompatible with this program!\n";

		return false;
	}

	// guess size of move array:
	size_t _numElements;
	{
		std::vector<std::string> tokens = INI::tokenize(lines.at(iLine), "\t");
		if (!INI::checkRangeB(tokens.size(), (size_t)2, tokens.max_size())) { return false; }

		if (!INI::setArgAndPrintError("moves numElements", tokens.at(1), _numElements, iLine, 1)) { return false; }

		if (!INI::checkRangeB(_numElements, (size_t)0, (size_t)MAXMOVES)) { return false; }
	}

	// ignore fluff line
	iLine+=2;

	// make sure number of lines in the file is correct for the number of moves we were given:
	if (!INI::checkRangeB(lines.size() - iLine, (size_t)_numElements, (size_t)MAXMOVES)) { return false; }
	getMoves().resize(_numElements);

	std::vector<std::string> mismatchedTypes;

	for (size_t iMove = 0; iMove < getMoves().size(); ++iMove, ++iLine)
	{
		std::vector<std::string> tokens = tokenize(lines.at(iLine), "\t");
		move& cMove = getMoves()[iMove];
		if (tokens.size() != 41)
		{
			std::cerr << "ERR " << __FILE__ << "." << __LINE__ << 
				": move inputStream has malformed line #" << iLine << 
				" with " << tokens.size() << " values!\n";
			return false;
		}
		cMove.lostChild = false;

		// move name
		size_t iToken = 0;
		cMove.setName(tokens.at(iToken));


		// move type
		iToken = 1;
		{
			size_t iType = orphanCheck(getTypes(), &mismatchedTypes, tokens.at(iToken));
			if (iType == SIZE_MAX) 
			{ 
				//incorrectArgs("type", iMove, iToken);
				cMove.cType = NULL;
				cMove.lostChild = true;
				//return false;
			} //orphan!
			else { cMove.cType = &getTypes()[iType]; }
		}

		// move base PP
		iToken = 2;
		{
			uint32_t currentPP;
			if (!setArg(tokens.at(iToken), currentPP)) { incorrectArgs("currentPP", iLine, iToken); return false; }
			checkRangeB(currentPP, (uint32_t)5, (uint32_t)40);
			cMove.PP = currentPP;
		}

		// move primary accuracy
		iToken = 3;
		if (tokens.at(iToken).compare("---") == 0)
		{ cMove.primaryAccuracy = UINT8_MAX; }
		else
		{
			uint32_t cPrimaryAccuracy;
			if (!setArg(tokens.at(iToken), cPrimaryAccuracy)) { incorrectArgs("cPrimaryAccuracy", iLine, iToken); return false; }
			checkRangeB(cPrimaryAccuracy, (uint32_t)1, (uint32_t)100);
			cMove.primaryAccuracy = cPrimaryAccuracy;
		}

		// move power
		iToken = 4;
		if (tokens.at(iToken).compare("Var") == 0 || tokens.at(iToken).compare("---") == 0)
		{ cMove.power = 0; }
		else
		{ 
			uint32_t cPower;
			if (!setArg(tokens.at(iToken), cPower)) { incorrectArgs("cPower", iLine, iToken); return false; }
			checkRangeB(cPower, (uint32_t)0, (uint32_t)254);
			cMove.power = cPower;
		}

		// move damage type
		iToken = 5;
		{
			uint32_t cDamageType;
			if (!setArg(tokens.at(iToken), cDamageType)) { incorrectArgs("cDamageType", iLine, iToken); return false; }
			checkRangeB(cDamageType, (uint32_t)0, (uint32_t)3);
			cMove.damageType = cDamageType;
		}

		// move target
		iToken = 6;
		if (tokens.at(iToken).compare("Var") == 0 || tokens.at(iToken).compare("---") == 0)
		{ cMove.target = -1; }
		else
		{ 
			uint32_t cTarget;
			if (!setArg(tokens.at(iToken), cTarget)) { incorrectArgs("cTarget", iLine, iToken); return false; }
			checkRangeB(cTarget, (uint32_t)0, (uint32_t)7);
			cMove.target = cTarget;
		}

		// move priority
		iToken = 7;
		{
			int32_t cPriority;
			if (!setArg(tokens.at(iToken), cPriority)) { incorrectArgs("cPriority", iLine, iToken); return false; }
			checkRangeB(cPriority, (int32_t)-5, (int32_t)5);
			cMove.priority = cPriority;
		}
		
		// buffs
		iToken = 8;
		for (unsigned int iBuff = 0; iBuff < 9; iBuff++)
		{
			// buff, self
			int32_t tempBuff;
			if (!setArg(tokens.at(iToken + iBuff), tempBuff)) { incorrectArgs("buff", iLine, iToken + iBuff); return false; }
			checkRangeB(tempBuff, (int32_t)-12, (int32_t)12);
			cMove.selfBuff[iBuff] = tempBuff;
		}

		// move secondary accuracy
		iToken = 17;
		if (tokens.at(iToken).compare("---") == 0)
		{ cMove.secondaryAccuracy = -1; }
		else
		{ 
			uint32_t cSecondaryAccuracy;
			if (!setArg(tokens.at(iToken), cSecondaryAccuracy)) { incorrectArgs("cSecondaryAccuracy", iLine, iToken); return false; }
			checkRangeB(cSecondaryAccuracy, (uint32_t)1, (uint32_t)100);
			cMove.secondaryAccuracy = cSecondaryAccuracy;
		}

		// debuffs
		iToken = 18;
		for (size_t iBuff = 0; iBuff < 9; iBuff++)
		{
			// debuff, target
			int32_t tempBuff;
			if (!setArg(tokens.at(iToken + iBuff), tempBuff)) { incorrectArgs("debuff", iLine, iToken + iBuff); return false; }
			checkRangeB(tempBuff, (int32_t)-12, (int32_t)12);
			cMove.targetDebuff[iBuff] = tempBuff;
		}

		uint32_t ailment;
		cMove.targetAilment = AIL_NV_NONE;

		// ailment,target,burn
		iToken = 27;
		if (setArg(tokens.at(iToken), ailment))
		{
			checkRangeB(ailment, (uint32_t)0, (uint32_t)1);
			if (ailment == 1) { cMove.targetAilment = AIL_NV_BURN; goto endAilment; }
		}
		else { incorrectArgs("AIL_NV_BURN", iLine, iToken); return false; }

		// ailment,target,Freeze
		iToken = 28;
		if (setArg(tokens.at(iToken), ailment))
		{
			checkRangeB(ailment, (uint32_t)0, (uint32_t)1);
			if (ailment == 1) { cMove.targetAilment = AIL_NV_FREEZE; goto endAilment; }
		}
		else { incorrectArgs("AIL_NV_FREEZE", iLine, iToken); return false; }

		// ailment,target,paralysis
		iToken = 29;
		if (setArg(tokens.at(iToken), ailment))
		{
			checkRangeB(ailment, (uint32_t)0, (uint32_t)1);
			if (ailment == 1) { cMove.targetAilment = AIL_NV_PARALYSIS; goto endAilment; }
		}
		else { incorrectArgs("AIL_NV_PARALYSIS", iLine, iToken); return false; }

		// ailment,target,poison
		iToken = 30;
		if (setArg(tokens.at(iToken), ailment))
		{
			checkRangeB(ailment, (uint32_t)0, (uint32_t)2);
			if (ailment == 1) { cMove.targetAilment = AIL_NV_POISON; goto endAilment; }
			else if(ailment == 2) { cMove.targetAilment = AIL_NV_POISON_TOXIC; goto endAilment; }
		}
		else { incorrectArgs("AIL_NV_POISON/AIL_NV_POISON_TOXIC", iLine, iToken); return false; }

		// ailment,target,sleep
		iToken = 31;
		if (setArg(tokens.at(iToken), ailment))
		{
			checkRangeB(ailment, (uint32_t)0, (uint32_t)1);
			if (ailment == 1) { cMove.targetAilment = AIL_NV_SLEEP; goto endAilment; }
		}
		else { incorrectArgs("AIL_NV_SLEEP", iLine, iToken); return false; }

		endAilment:

		cMove.targetVolatileAilment = AIL_V_NONE;
		// volatileAilment,target,confusion
		iToken = 32;
		if (setArg(tokens.at(iToken), ailment))
		{
			checkRangeB(ailment, (uint32_t)0, (uint32_t)1);
			if (ailment == 1) { cMove.targetVolatileAilment = AIL_V_CONFUSED; goto endVolatile; }
		}
		else { incorrectArgs("AIL_V_CONFUSED", iLine, iToken); return false; }

		// volatileAilment,target,flinch
		iToken = 33;
		if (setArg(tokens.at(iToken), ailment))
		{
			checkRangeB(ailment, (uint32_t)0, (uint32_t)1);
			if (ailment == 1) { cMove.targetVolatileAilment = AIL_V_FLINCH; goto endVolatile; }
		}
		else { incorrectArgs("AIL_V_FLINCH", iLine, iToken); return false; }

		// volatileAilment,target,identify
		iToken = 34;
		if (setArg(tokens.at(iToken), ailment))
		{
			checkRangeB(ailment, (uint32_t)0, (uint32_t)1);
			if (ailment == 1) { cMove.targetVolatileAilment = 110; goto endVolatile; }
		}
		else { incorrectArgs("AIL_V_IDENTIFY", iLine, iToken); return false; }

		// volatileAilment,target,infatuation
		iToken = 35;
		if (setArg(tokens.at(iToken), ailment))
		{
			checkRangeB(ailment, (uint32_t)0, (uint32_t)1);
			if (ailment == 1) { cMove.targetVolatileAilment = AIL_V_INFATUATED; goto endVolatile; }
		}
		else { incorrectArgs("AIL_V_INFATUATED", iLine, iToken); return false; }

		// volatileAilment,target,lock on
		iToken = 36;
		if (setArg(tokens.at(iToken), ailment))
		{
			checkRangeB(ailment, (uint32_t)0, (uint32_t)1);
			if (ailment == 1) { cMove.targetVolatileAilment = 111; goto endVolatile; }
		}
		else { incorrectArgs("AIL_V_LOCKON", iLine, iToken); return false; }

		// volatileAilment,target,nightmare
		iToken = 37;
		if (setArg(tokens.at(iToken), ailment))
		{
			checkRangeB(ailment, (uint32_t)0, (uint32_t)1);
			if (ailment == 1) { cMove.targetVolatileAilment = 112; goto endVolatile; }
		}
		else { incorrectArgs("AIL_V_NIGHTMARE", iLine, iToken); return false; }

		// volatileAilment,target,partial trap
		iToken = 38;
		if (setArg(tokens.at(iToken), ailment))
		{
			checkRangeB(ailment, (uint32_t)0, (uint32_t)1);
			if (ailment == 1) { cMove.targetVolatileAilment = 113; goto endVolatile; }
		}
		else { incorrectArgs("AIL_V_PARTIALTRAP", iLine, iToken); return false; }

		endVolatile:

		//hasScript
		iToken = 39;
		if (tokens.at(iToken).compare("---") == 0)
		{ cMove.setHasNoPlugins(); }

		//description
		iToken = 40;
		if (tokens.at(iToken).compare("---") == 0)
		{ cMove.description.clear(); }
		else
		{
			size_t tokenLength = tokens.at(iToken).size();
			size_t offset = 0;
			//check for quotations, and if they exist remove them
			if (tokens.at(iToken)[0] == '"' && tokens.at(iToken)[tokenLength-1] == '"')
			{
				offset = 1;
			}
			
			cMove.description = std::string(tokens.at(iToken).substr(offset, tokenLength - offset));
		}
		if (verbose >= 6) std::cout << "\tLoaded move " << iMove << "-\"" << cMove.getName() << "\"\n";

	} //end of per-move

	if (mismatchedTypes.size() != 0)
	{
		if (verbose >= 5) std::cerr << "WAR " << __FILE__ << "." << __LINE__ << 
			": move inputStream - " << mismatchedTypes.size() << " Orphaned move-types!\n";
		if (verbose >= 6)
		{
			for (size_t indexOrphan = 0; indexOrphan < mismatchedTypes.size(); indexOrphan++)
			{
				std::cout << "\tOrphaned type \"" << mismatchedTypes.at(indexOrphan) << "\"\n";
			}
		}
	}

	return true;
} // end of inputMoves





bool pokedex::inputPlugins(const std::string& input_pluginFolder)
{
#ifndef _DISABLEPLUGINS
	std::vector<std::string> mismatchedItems;
	std::vector<std::string> mismatchedAbilities;
	std::vector<std::string> mismatchedMoves;
	//std::vector<std::string> mismatchedGears; // engine components
	std::vector<std::string> mismatchedCategories;
	size_t numOverwritten = 0;
	size_t numExtensions = 0;
	size_t numPluginsLoaded = 0;
	size_t numPluginsTotal = 0;

	boost::filesystem::path pluginLocation(input_pluginFolder);
	
	// determine if folder exists:
	if (!boost::filesystem::exists(pluginLocation) || !boost::filesystem::is_directory(pluginLocation))
	{
		std::cerr << "ERR " << __FILE__ << "." << __LINE__ << 
			": A script folder was not found at location \"" << input_pluginFolder << "\"!\n";
		return false;
	}

	// determine if folder is a directory:
	if (!boost::filesystem::is_directory(pluginLocation))
	{
		std::cerr << "ERR " << __FILE__ << "." << __LINE__ << 
			": \"" << input_pluginFolder << "\" is not a directory!\n";
		return false;
	}

	// iterate through all files in moves directory:
	for ( boost::filesystem::directory_iterator iPlugin(pluginLocation), endPlugin; iPlugin != endPlugin; ++iPlugin)
	{
		// ignore directories
		if (boost::filesystem::is_directory(*iPlugin)) { continue; }

		// make sure extension is that of a plugin:
#if defined(WIN32) || defined(_CYGWIN)
		if (iPlugin->path().extension().compare(".dll") != 0) { continue; }
#else // probably linux
		if (iPlugin->path().extension().compare(".so") != 0) { continue; }
#endif

		if (verbose >= 6)
		{
			std::cout << "Loading plugin at " << *iPlugin << "...\n";
		}

		numPluginsTotal++;
		shared_library* cPlugin = new shared_library(iPlugin->path().file_string(), false);

		// attempt to load plugin:
		if (!cPlugin->open())
		{
			std::cerr << "ERR " << __FILE__ << "." << __LINE__ << 
				": plugin \"" << *iPlugin <<
				"\" could not be loaded:!\n";
			continue;
		}

		// attempt to find function which enumerates scripts within this plugin:
		//regExtension_type registerExtensions = NULL;
		regExtension_type registerExtensions(cPlugin->get<bool, const pokedex&, std::vector<plugin>&>("registerExtensions"));
		if (!registerExtensions)
		{
			std::cerr << "ERR " << __FILE__ << "." << __LINE__ << 
				": could not find registerExtensions method in plugin \"" << *iPlugin << 
				"\"!\n";
			// close faulty module:
			if (!cPlugin->close())
			{
				std::cerr << "ERR " << __FILE__ << "." << __LINE__ << 
					": FATAL - could not unregister unacceptable plugin \"" << *iPlugin << 
					"\"!\n";
				return false;
			}
			continue;
		}

		bool success = registerPlugin(
			(regExtension_rawType)registerExtensions.functor.func_ptr, 
			&numExtensions, 
			&numOverwritten, 
			&mismatchedItems, 
			&mismatchedAbilities, 
			&mismatchedMoves, 
			&mismatchedCategories);

		if (!success)
		{
			if (!cPlugin->close())
			{
				std::cerr << "ERR " << __FILE__ << "." << __LINE__ << 
					": FATAL - could not unregister unacceptable plugin \"" << *iPlugin << 
					"\"!\n";
				return false;
			}
			continue;
		}

		// push back successful load of plugin
		getPlugins().push_back(cPlugin);
		numPluginsLoaded++;
	}// endof foreach plugin

	// print orphans:
	// print mismatched items
	if (mismatchedItems.size() != 0)
	{
		std::cerr << "ERR " << __FILE__ << "." << __LINE__ << 
			": \"" << input_pluginFolder <<
			"\" - " << mismatchedItems.size() << " Orphaned plugin-items!\n";
		if (verbose >= 4)
		{
			for (size_t iOrphan = 0; iOrphan < mismatchedItems.size(); iOrphan++)
			{
				std::cerr << "\tOrphaned item \"" << mismatchedItems.at(iOrphan) << "\"\n";
			}
		}
	}

	// print mismatched abilities
	if (mismatchedAbilities.size() != 0)
	{
		std::cerr << "ERR " << __FILE__ << "." << __LINE__ << 
			": \"" << input_pluginFolder <<
			"\" - " << mismatchedAbilities.size() << " Orphaned plugin-abilities!\n";
		if (verbose >= 5)
		{
			for (size_t iOrphan = 0; iOrphan < mismatchedAbilities.size(); iOrphan++)
			{
				std::cerr << "\tOrphaned ability \"" << mismatchedAbilities.at(iOrphan) << "\"\n";
			}
		}
	}

	// print mismatched moves
	if (mismatchedMoves.size() != 0)
	{
		std::cerr << "ERR " << __FILE__ << "." << __LINE__ << 
			": \"" << input_pluginFolder <<
			"\" - " << mismatchedMoves.size() << " Orphaned plugin-moves!\n";
		if (verbose >= 5)
		{
			for (size_t iOrphan = 0; iOrphan < mismatchedMoves.size(); iOrphan++)
			{
				std::cerr << "\tOrphaned move \"" << mismatchedMoves.at(iOrphan) << "\"\n";
			}
		}
	}

	// print mismatched categories
	if (mismatchedCategories.size() != 0)
	{
		std::cerr << "ERR " << __FILE__ << "." << __LINE__ << 
			": \"" << input_pluginFolder <<
			"\" - " << mismatchedCategories.size() << " Orphaned plugin-categories!\n";
		if (verbose >= 5)
		{
			for (size_t iOrphan = 0; iOrphan < mismatchedMoves.size(); iOrphan++)
			{
				std::cerr << "\tOrphaned category \"" << mismatchedCategories.at(iOrphan) << "\"\n";
			}
		}
	}

	if (verbose >= 6)
	{
		std::cerr << "INF " << __FILE__ << "." << __LINE__ << 
			" Successfully loaded  " << numPluginsLoaded << 
			" of " << numPluginsTotal <<
			" plugins, loaded " << numExtensions << 
			" ( " << numOverwritten << " overwritten ) extensions!\n";
	}

#endif /* _DISABLEPLUGINS */
	return true;
} // endOf inputScript

bool pokedex::registerPlugin(
	regExtension_rawType registerExtensions,
	size_t* _numExtensions,
	size_t* _numOverwritten,
	std::vector<std::string>* _mismatchedItems,
	std::vector<std::string>* _mismatchedAbilities,
	std::vector<std::string>* _mismatchedMoves,
	std::vector<std::string>* _mismatchedCategories)
{
	assert(registerExtensions != NULL);

	std::vector<std::string> mismatchedItems;
	std::vector<std::string> mismatchedAbilities;
	std::vector<std::string> mismatchedMoves;
	std::vector<std::string> mismatchedCategories;
	size_t numOverwritten = 0;
	size_t numExtensions = 0;

	// register function handles:
	std::vector<plugin> collectedPlugins;
	if (!registerExtensions(*this, collectedPlugins))
	{
		std::cerr << "ERR " << __FILE__ << "." << __LINE__ << 
			": engine plugin was not able to generate a list of valid plugins!\n";
		return false;
	}

	// load in all functions from this module:
	for (size_t iCPlugin = 0; iCPlugin != collectedPlugins.size(); ++iCPlugin)
	{
		pluggableInterface* element = NULL;

		// find which element this plugin refers to
		plugin& cCPlugin = collectedPlugins[iCPlugin];
		if (cCPlugin.getCategory().compare(MOVE_PLUGIN) == 0)
		{
			size_t iMove = orphanCheck(getMoves(), &mismatchedMoves, cCPlugin.getName());
			if (iMove == SIZE_MAX) { continue; } // orphan!
			element = &getMoves()[iMove];
		}
		else if (cCPlugin.getCategory().compare(ABILITY_PLUGIN) == 0)
		{
			size_t iAbility = orphanCheck(getAbilities(), &mismatchedAbilities, cCPlugin.getName());
			if (iAbility == SIZE_MAX) { continue; } // orphan!
			element = &getAbilities()[iAbility];
		}
		else if (cCPlugin.getCategory().compare(ITEM_PLUGIN) == 0)
		{
			size_t iItem = orphanCheck(getItems(), &mismatchedItems, cCPlugin.getName());
			if (iItem == SIZE_MAX) { continue; } // orphan!
			element = &getItems()[iItem];
		}
		else if (cCPlugin.getCategory().compare(ENGINE_PLUGIN) == 0)
		{
			element = &getExtensions();
		}
		else // unknown category:
		{
			mismatchedCategories.push_back(cCPlugin.getCategory());
			continue;
		}

		bool overwritten = false;
		// register plugin to its move/ability/item/engine:
		if (element != NULL)
		{
			overwritten = element->registerPlugin(cCPlugin);
		}

		// if plugin overwrote a plugin that was previously installed:
		if (overwritten)
		{
			if (verbose >= 5)
			{
				std::cerr << "WAR " << __FILE__ << "." << __LINE__ << 
					": plugin for [" << cCPlugin.getCategory() <<
					"][" << cCPlugin.getName() << "] -- overwriting previously defined plugin!\n";
			}
			numOverwritten++;
		}
		numExtensions++;
	} // endOf foreach plugin element

	// add mismatched elements, if the user wants them:
	if (_mismatchedItems != NULL) { _mismatchedItems->insert(_mismatchedItems->end(), mismatchedItems.begin(), mismatchedItems.end()); }
	if (_mismatchedAbilities != NULL) { _mismatchedAbilities->insert(_mismatchedAbilities->end(), mismatchedAbilities.begin(), mismatchedAbilities.end()); }
	if (_mismatchedMoves != NULL) { _mismatchedMoves->insert(_mismatchedMoves->end(), mismatchedMoves.begin(), mismatchedMoves.end()); }
	if (_mismatchedCategories != NULL) { _mismatchedCategories->insert(_mismatchedCategories->end(), mismatchedCategories.begin(), mismatchedCategories.end()); }

	if (_numOverwritten != NULL) { *_numOverwritten += numOverwritten; }
	if (_numExtensions != NULL) { *_numExtensions += numExtensions; }

	return true;
}; // endOf registerPlugin
