
//#define PKAI_EXPORT
#include "../inc/pokemon_nonvolatile.h"
//#undef PKAI_EXPORT

#include <algorithm>

//#define PKAI_STATIC
#include "../inc/pokedex.h"
#include "../inc/pokemon_volatile.h"
#include "../inc/team_volatile.h"
#include "../inc/pokemon_base.h"
#include "../inc/nature.h"
#include "../inc/ability.h"
#include "../inc/item.h"
//#undef PKAI_STATIC
#include "../inc/init_toolbox.h"
#include "../inc/orphan.h"

using namespace orphan;

boost::array< boost::array<fpType, 13>, 3> pokemon_nonvolatile::aFV_base;

pokemon_nonvolatile::pokemon_nonvolatile() 
	: name(),
	signature<pokemon_nonvolatile, POKEMON_NONVOLATILE_DIGESTSIZE>(),
	base(pokemon_base::no_base),
	chosenAbility(ability::no_ability), 
	chosenNature(nature::no_nature), 
	actions(),
	numMoves(0),
	initialItem(-1),
	level(0), 
	sex(SEX_NEUTER)
{	
	// zero IV and EV
	IV.assign(0);
	EV.assign(0);
	
	// zero FV
	for (size_t iFV = 0; iFV < FV_base.size(); iFV++)
	{
		FV_base[iFV].assign(0);
	}
	
}





pokemon_nonvolatile::pokemon_nonvolatile(const pokemon_nonvolatile& orig) 
	: name(orig),
	signature<pokemon_nonvolatile, POKEMON_NONVOLATILE_DIGESTSIZE>(orig),
	base(orig.base),
	chosenAbility(orig.chosenAbility), 
	chosenNature(orig.chosenNature), 
	actions(orig.actions),
	numMoves(orig.numMoves),
	initialItem(orig.initialItem),
	level(orig.level), 
	sex(orig.sex),
	IV(orig.IV),
	EV(orig.EV),
	FV_base(orig.FV_base)
{
}

pokemon_nonvolatile& pokemon_nonvolatile::operator=(const pokemon_nonvolatile& other)
{
	// identity theorem - simply return what we have now if equal address
	if (this == &other) { return *this; } 
	
	name::operator=(other);
	signature<pokemon_nonvolatile, POKEMON_NONVOLATILE_DIGESTSIZE>::operator=(other);
	base = other.base;
	chosenAbility = other.chosenAbility;
	chosenNature = other.chosenNature;
	actions = other.actions;
	numMoves = other.numMoves;
	initialItem = other.initialItem;
	level = other.level;
	sex = other.sex;
	IV = other.IV;
	EV = other.EV;
	FV_base = other.FV_base;
	
	return *this;
}





void pokemon_nonvolatile::initialize()
{
	for (size_t iAction = 0; iAction < getNumMoves(); iAction++)
	{
		move_nonvolatile& cMove = getMove(iAction);

		// do not initialize if the move_nonvolatile object does not reference a valid move
		if (!cMove.moveExists()) { continue; }

		cMove.initialize(*this);
	}
	initFV();
};





void pokemon_nonvolatile::uninitialize()
{
	for (size_t iAction = 0; iAction < getNumMoves(); iAction++)
	{
		move_nonvolatile& cMove = getMove(iAction);

		// do not initialize if the move_nonvolatile object does not reference a valid move
		if (!cMove.moveExists()) { continue; }

		cMove.uninitialize(*this);
	}
}





void pokemon_nonvolatile::createDigest_impl(boost::array<uint8_t, POKEMON_NONVOLATILE_DIGESTSIZE>& digest) const
{
	digest.assign(0);

	boost::array<bool, 4> hashedMoves;
	hashedMoves.assign(false);
	size_t iDigest = 0;

	// hash by a useful order:
	for (size_t iOrder = 0, iSize = getNumMoves(); iOrder < iSize; ++iOrder)
	{
		size_t iBestMove = SIZE_MAX;
		const move_nonvolatile* bestMove = NULL;
		for (size_t iAction = 0; iAction != iSize; ++iAction)
		{
			const move_nonvolatile& cMove = getMove(iAction + AT_MOVE_0);

			// don't hash a move that has already been hashed:
			if (hashedMoves[iAction] == true) { continue; }

			// if no move has been selected yet, select the first move:
			if (bestMove == NULL) { bestMove = &cMove; iBestMove = iAction; continue; }

			// if bestMove appears later in the array of base moves than does cMove: (higher in alphabetical order)
			if (&bestMove->getBase() > &cMove.getBase()) { bestMove = &cMove; iBestMove = iAction; }
		}

		// no more moves to be hashed
		if (bestMove == NULL) { break; }

		hashedMoves[iBestMove] = true;

		// hash action:
		boost::array<uint8_t, MOVE_NONVOLATILE_DIGESTSIZE> bMoveDigest;
		bestMove->createDigest(bMoveDigest);

		// copy action to pokemon digest:
		pack(bMoveDigest, digest, iDigest);
	} // endOf foreach moveOrdered
	iDigest = MOVE_NONVOLATILE_DIGESTSIZE * 4;

	
	if (pokemonExists())
	{
		// pack first 20 characters of name:
		getBase().getName().copy((char *)(digest.c_array() + iDigest), 20, 0);
	}
	iDigest += 20;

	if (abilityExists())
	{ // pack 20 characters of ability:
		getAbility().getName().copy((char *)(digest.c_array() + iDigest), 20, 0);
	}
	iDigest += 20;

	if (natureExists())
	{ // pack 20 characters of nature:
		getNature().getName().copy((char *)(digest.c_array() + iDigest), 20, 0);
	}
	iDigest += 20;

	if (hasInitialItem())
	{
		// pack 20 characters of item, if it exists:
		getInitialItem().getName().copy((char *)(digest.c_array() + iDigest), 20, 0);
	}
	iDigest += 20;

	// pack level:
	pack(level, digest, iDigest);

	// pack sex:
	pack(sex, digest, iDigest);

	// pack IVEV:
	for (size_t iIEV = 0; iIEV < IV.size(); ++iIEV)
	{
		pack(IV[iIEV], digest, iDigest);
		pack(EV[iIEV], digest, iDigest);
	}

	assert(iDigest == POKEMON_NONVOLATILE_DIGESTSIZE);
};





bool pokemon_nonvolatile::pokemonExists() const
{
	return (base!=pokemon_base::no_base)?true:false;
};





bool pokemon_nonvolatile::abilityExists() const
{
	return (chosenAbility!=ability::no_ability)?true:false;
};





void pokemon_nonvolatile::setAbility(const ability& _chosenAbility)
{
	assert(pokemonExists());
	assert(_chosenAbility.isImplemented());
	assert(std::binary_search(getBase().abilities.begin(), getBase().abilities.end(), &_chosenAbility));

	chosenAbility = &_chosenAbility;
};





void pokemon_nonvolatile::setNoAbility()
{
	chosenAbility = ability::no_ability;
};





void pokemon_nonvolatile::setNoNature()
{
	chosenNature = nature::no_nature;
};





bool pokemon_nonvolatile::natureExists() const
{
	return (chosenNature!=nature::no_nature)?true:false;
};





void pokemon_nonvolatile::setNature(const nature& _chosenNature)
{
	assert(((size_t)(&_chosenNature - &pkdex->getNatures().front())) < pkdex->getNatures().size());
	chosenNature = &_chosenNature;
};





void pokemon_nonvolatile::setNoInitialItem()
{
	initialItem = UINT8_MAX;
};





void pokemon_nonvolatile::setInitialItem(const item& _chosenItem)
{
	size_t iItem = &_chosenItem - &pkdex->getItems().front();
	assert (iItem < pkdex->getItems().size());
	assert(_chosenItem.isImplemented());
	initialItem = (uint8_t) iItem;
}





bool pokemon_nonvolatile::hasInitialItem() const
{
	return initialItem != UINT8_MAX;
}





const item& pokemon_nonvolatile::getInitialItem() const
{
	return pkdex->getItems()[initialItem];
}





bool pokemon_nonvolatile::isLegalAdd(const move_nonvolatile& candidate) const
{
	if ( !candidate.moveExists() ) { return false; }
	return isLegalAdd(candidate.getBase());
}






bool pokemon_nonvolatile::isLegalSet(size_t iAction, const move_nonvolatile& candidate) const
{
	if ( !candidate.moveExists() ) { return false; }
	return isLegalSet(iAction, candidate.getBase());
}





bool pokemon_nonvolatile::isLegalAdd(const move& candidate) const
{
	if ((getNumMoves() + 1) > getMaxNumMoves()) { return false; }
	return isLegalSet(SIZE_MAX, candidate);
}






bool pokemon_nonvolatile::isLegalSet(size_t iAction, const move& candidate) const
{
	size_t iPosition = iAction - AT_MOVE_0;
	if (!pokemonExists()) { return false; }
	if ((iPosition != SIZE_MAX) && (iPosition >= getNumMoves()) ) { return false; }
	if ((candidate.lostChild == true) || (candidate.isImplemented() == false)) { return false; }
	for (size_t iMove = 0; iMove != getNumMoves(); ++iMove)
	{
		if (iPosition == iMove) { continue; }
		if (&getMove_base(AT_MOVE_0 + iMove) == &candidate) { return false; }
	}

	size_t candidateIndex = (&candidate - &pkdex->getMoves().front());
	const std::vector<size_t>& cMovelist = getBase().movelist;
	if (!std::binary_search(cMovelist.begin(), cMovelist.end(), candidateIndex)) { return false; }

	return true;
}





void pokemon_nonvolatile::addMove(const move_nonvolatile& _cMove)
{
	assert(isLegalAdd(_cMove));

	actions[numMoves] = _cMove;
	numMoves++;
}




move_nonvolatile& pokemon_nonvolatile::getMove(size_t index)
{
	switch(index)
	{
		default:
			assert(false && "attempted to get volatile move of non-move action");
		case AT_MOVE_STRUGGLE:
			return *move_nonvolatile::mNV_struggle;
		case AT_MOVE_0:
		case AT_MOVE_1:
		case AT_MOVE_2:
		case AT_MOVE_3:
			assert((index - AT_MOVE_0) < getNumMoves());
			return actions[index - AT_MOVE_0];
	}
};





const move_nonvolatile& pokemon_nonvolatile::getMove(size_t index) const
{
	switch(index)
	{
		default:
			assert(false && "attempted to get volatile move of non-move action");
		case AT_MOVE_STRUGGLE:
			return *move_nonvolatile::mNV_struggle;
		case AT_MOVE_0:
		case AT_MOVE_1:
		case AT_MOVE_2:
		case AT_MOVE_3:
			assert((index - AT_MOVE_0) < getNumMoves());
			return actions[index - AT_MOVE_0];
	}
};





void pokemon_nonvolatile::setMove(size_t iAction, const move_nonvolatile& _cMove)
{
	assert(isLegalSet(iAction - AT_MOVE_0, _cMove));
	getMove(iAction) = _cMove;
};





void pokemon_nonvolatile::removeMove(size_t iRemovedAction)
{
	// don't bother removing a move that doesn't exist
	if ((iRemovedAction - AT_MOVE_0) >= getNumMoves()) { return; }

	{
		move_nonvolatile& removedMove = getMove(iRemovedAction);

		// remove move:
		removedMove = move_nonvolatile();
	}

	// there's a "hole" in the contiguous move array now, so 
	// refactor moves above the removed move:
	for (size_t iSource = (iRemovedAction - AT_MOVE_0) + 1; iSource < getNumMoves(); iSource++)
	{
		move_nonvolatile& source = getMove(AT_MOVE_0 + iSource);

		for (size_t iNDestination = 0; iNDestination < iSource; iNDestination++)
		{
			size_t iDestination = (iSource - iNDestination - 1);

			move_nonvolatile& destination = actions[iDestination];

			// don't replace a move that exists already
			if (destination.moveExists()) { continue; }

			// perform copy
			destination = source;
			// delete source (no duplicates)
			source = move_nonvolatile();
			break;
		}
	}

	// update number of moves in move array:
	numMoves--;
};





void pokemon_nonvolatile::setFV(unsigned int targetFV)
{	

	// set default value:
	if (targetFV == FV_HITPOINTS)
	{
		unsigned int baseStat = base->baseStats[targetFV];
		unsigned int iv = IV[targetFV];
		unsigned int ev = EV[targetFV];
		
		FV_base[targetFV][STAGE0] = ((2 * baseStat + iv + (ev / 4)) * level / 100 + level + 10);
	}
	else if (targetFV == FV_ACCURACY || targetFV == FV_EVASION)
	{
		aFV_base[targetFV - 6][STAGE0] = 1.0; // 1.0
	}
	else if (targetFV == FV_CRITICALHIT)
	{
		// critical hit stage 1 is hardcoded
		aFV_base[targetFV - 6][STAGE0] =  0.0625; //(ACCURACY_EVASION_INTEGER * 0.0625);
	}
	else // for atk, spa, def, spd, spe
	{
		unsigned int baseStat = base->baseStats[targetFV];
		unsigned int iv = IV[targetFV];
		unsigned int ev = EV[targetFV];
		unsigned int natureModification = chosenNature->modTable[targetFV];
		
		unsigned int base_FV = ((((2 * baseStat + iv + (ev / 4)) * level / 100 + 5) * natureModification) / FPMULTIPLIER);
		FV_base[targetFV][STAGE0] = base_FV;
	}

	// set boosted values:
	for (size_t iBoost = 0; iBoost != 13; ++iBoost)
	{
		int boostStage = (int)iBoost - 6;

		if (boostStage == 0) { continue; } // don't modify base values

		if (targetFV == FV_HITPOINTS)
		{
			// hitpoints cannot be boosted
			FV_base[targetFV][iBoost] = FV_base[targetFV][STAGE0];
		}
		else if (targetFV == FV_ACCURACY || targetFV == FV_EVASION)
		{
			fpType boostNumerator = 1, boostDenominator = 1;
			if (boostStage >= 1)
			{
				boostNumerator = 3 + boostStage;
				boostDenominator = 3;
			}
			else // boostStage <= -1
			{
				boostNumerator = 3;
				boostDenominator = 3 - boostStage;
			}

			if (targetFV == FV_ACCURACY)
			{
				aFV_base[targetFV - 6][iBoost] = (aFV_base[targetFV - 6][STAGE0] * boostNumerator) / boostDenominator;
			}
			else // evasion is accuracy's modification flipped
			{
				aFV_base[targetFV - 6][iBoost] = (aFV_base[targetFV - 6][STAGE0] * boostDenominator) / boostNumerator;
			}
			
		}// endOf if FV_ACCURACY or FV_EVASION
		else if (targetFV == FV_CRITICALHIT)
		{
			// values of critical hit are hardcoded, and are always 0 when less than stage 0
			fpType boosted_FV;
		
			// values of critical hit hardcoded
			switch(boostStage)
			{
				default:
					boosted_FV = 0.0; // no critical hit possible
					break;
				case 0:
					boosted_FV = 0.0625; // ACCURACY_EVASION_INTEGER * .0625
					break;
				case 1:
					boosted_FV = 0.125; // ACCURACY_EVASION_INTEGER * .125
					break;
				case 2:
					boosted_FV = 0.25; // ACCURACY_EVASION_INTEGER * .25
					break;
				case 3:
					boosted_FV = (1.0/3.0); // ACCURACY_EVASION_INTEGER * .333
					break;
				case 4: // maximum stage for critical hit is 4
				case 5:
				case 6:
					boosted_FV = 0.5; // ACCURACY_EVASION_INTEGER * .5
					break;
			}
		
			aFV_base[targetFV - 6][iBoost] = boosted_FV;
		} // endOf if FV_CRITICALHIT
		else // for atk, spa, def, spd, spe
		{
			int32_t boostNumerator = 1, boostDenominator = 1;
			if (boostStage >= 1)
			{
				boostNumerator = 2 + boostStage;
				boostDenominator = 2;
			}
			else  // boostStage <= -1
			{
				boostNumerator = 2;
				boostDenominator = 2 - boostStage;
			}

			FV_base[targetFV][iBoost] = (FV_base[targetFV][STAGE0] * boostNumerator) / boostDenominator;
		} // endOf atk, spa, def, spd, spe
	} // endOf foreach boost stage
} // endOf setFV





void pokemon_nonvolatile::initFV()
{
	// generate final values for a pokemon
	for (unsigned int indexFV = 0; indexFV < 9; indexFV++)
	{
		setFV(indexFV);
	}
}





const move& pokemon_nonvolatile::getMove_base(size_t index) const
{
	switch(index)
	{
		case AT_MOVE_NOTHING:
		case AT_MOVE_CONFUSED:
		default:
			assert(false && "attempted to get base of non-move action");
			return *move::move_none;
		case AT_MOVE_STRUGGLE:
			return *move::move_struggle;
		case AT_MOVE_0:
		case AT_MOVE_1:
		case AT_MOVE_2:
		case AT_MOVE_3:
			return getMove(index).getBase();
	}
};




std::ostream& operator <<(std::ostream& os, const pokemon_nonvolatile& cPKNV)
{
	os << "\"" << cPKNV.getName() << "\"-\"" << cPKNV.getBase().getName();
	os << " " << (cPKNV.abilityExists()?cPKNV.getAbility().getName():"NO_ABILITY");
	os << " " << (cPKNV.hasInitialItem()?cPKNV.getInitialItem().getName():"NO_ITEM");
	for (size_t iAction = 0; iAction != cPKNV.getNumMoves(); ++iAction)
	{
		os << " " << cPKNV.getMove_base(iAction + AT_MOVE_0).getName();
	}
	os << "\n";

	return os;
}

std::ostream& operator <<(std::ostream& os, const pokemon_print& cP)
{
	os << "\"" << cP.cPokemon.getName() << "\"-\"" << cP.cPokemon.getBase().getName() << "\" " << cP.currentPokemon.getHP() << "/" << cP.cTeam.cGetFV_boosted(cP.cPokemon, FV_HITPOINTS);

	// non-volatile ailments
	switch(cP.currentPokemon.getStatusAilment())
	{
		case AIL_NV_BURN:
			os << " BRN";
			break;
		case AIL_NV_FREEZE:
			os << " FRZ";
			break;
		case AIL_NV_PARALYSIS:
			os << " PAR";
			break;
		case AIL_NV_POISON_TOXIC:
			os << " PST";
			break;
		case AIL_NV_POISON:
			os << " PSN";
			break;
		case AIL_NV_REST_1T:
		case AIL_NV_REST_2T:
		case AIL_NV_REST_3T:
		case AIL_NV_SLEEP_4T:
		case AIL_NV_SLEEP_3T:
		case AIL_NV_SLEEP_2T:
		case AIL_NV_SLEEP_1T:
			os << " SLP";
			break;
		case AIL_NV_NONE:
		default:
			break;
	}
	
	// boosts:
	if (cP.currentPokemon == cP.cTeam.getPKV())
	{
		// volatile ailments:
		// target confused:
		if (cP.cTeam.getVolatile().confused > 0)
		{
			os << " (CNFSD)";
		}
		// target infatuated:
		if (cP.cTeam.getVolatile().infatuate > 0)
		{
			os << " (INFAT)";
		}
		/*// target flinched:
		sA_V = AIL_V_FLINCH;
		if (cP.currentPokemon.getVolatileStatusCondition(cP.cPokemon, sA_V))
		{
			os << " (FLNCH)";
		}
		// target flinched last turn:
		sA_V = AIL_V_FLINCHED;
		if (cP.currentPokemon.getVolatileStatusCondition(cP.cPokemon, sA_V))
		{
			os << " (FLNCH)";
		}*/

		os << std::showpos; // show the + or -
		if (cP.cTeam.cGetBoost(FV_ATTACK) != 0)
		{
			os << " " << cP.cTeam.cGetBoost(FV_ATTACK) << "atk";
		}
		if (cP.cTeam.cGetBoost(FV_SPATTACK) != 0)
		{
			os << " " << cP.cTeam.cGetBoost(FV_SPATTACK) << "spa";
		}
		if (cP.cTeam.cGetBoost(FV_DEFENSE) != 0)
		{
			os << " " << cP.cTeam.cGetBoost(FV_DEFENSE) << "def";
		}
		if (cP.cTeam.cGetBoost(FV_SPDEFENSE) != 0)
		{
			os << " " << cP.cTeam.cGetBoost(FV_SPDEFENSE) << "spd";
		}
		if (cP.cTeam.cGetBoost(FV_SPEED) != 0)
		{
			os << " " << cP.cTeam.cGetBoost(FV_SPEED) << "spe";
		}
		if (cP.cTeam.cGetBoost(FV_EVASION) != 0)
		{
			os << " " << cP.cTeam.cGetBoost(FV_EVASION) << "eva";
		}
		if (cP.cTeam.cGetBoost(FV_ACCURACY) != 0)
		{
			os << " " << cP.cTeam.cGetBoost(FV_ACCURACY) << "acc";
		}
		if (cP.cTeam.cGetBoost(FV_CRITICALHIT) != 0)
		{
			os << " " << cP.cTeam.cGetBoost(FV_CRITICALHIT) << "crt";
		}
		os << std::noshowpos; // stop showing + or -
	}
	
	os << "\n";
	
	return os;
}



static const std::string header = "PKAIP0";

void pokemon_nonvolatile::output(std::ostream& oFile, bool printHeader) const
{
	// header:
	if (printHeader)
	{
		oFile << header << "\t";
	}

	/* PKAIP0 <nickname> <species> <level> <item> <gender> <ability> <nature> <hp.type> <hp.dmg> <move 1> <move 2> <move 3> <move 4> <atk.iv> <spatck.iv> <def.iv> <spdef.iv> <spd.iv> <hp.iv> <atk.ev> <spatck.ev> <def.ev> <spdef.ev> <spd.ev> <hp.ev> */
	oFile 
		<< getName() <<
		"\t" << (pokemonExists()?getBase().getName():"NONE") <<
		"\t" << getLevel() <<
		"\t" << (hasInitialItem()?getInitialItem().getName():"NONE") <<
		"\t" << getSex() <<
		"\t" << (abilityExists()?getAbility().getName():"NONE") <<
		"\t" << getNature().getName() <<
		"\t";

	// moves:
	for (size_t iMove = 0; iMove != 4; ++iMove)
	{
		oFile << ((iMove < getNumMoves())?getMove_base(iMove).getName():"NONE") << "\t";
	}

	// IVs:
	for (size_t iIV = 0; iIV != 6; ++iIV)
	{
		oFile << getIV(iIV) << "\t";
	}

	// EVs:
	for (size_t iEV = 0; iEV != 6; ++iEV)
	{
		oFile << getEV(iEV) << "\t";
	}

	// end of line
	oFile << "\n";
} // endOf outputPokemon

bool pokemon_nonvolatile::input(
		const std::vector<std::string>& lines, 
		size_t& iLine, 
		std::vector<std::string>* mismatchedPokemon,
		std::vector<std::string>* mismatchedItems,
		std::vector<std::string>* mismatchedAbilities,
		std::vector<std::string>* mismatchedNatures,
		std::vector<std::string>* mismatchedMoves)
{
	// are the enough lines in the input stream:
	if ((lines.size() - iLine) < 1U)
	{
		std::cerr << "ERR " << __FILE__ << "." << __LINE__ << 
			": unexpected end of input stream at line " << iLine << "!\n";
		return false; 
	}

	// compare pokemon_nonvolatile header:
	if (lines.at(iLine).compare(0, header.size(), header) != 0)
	{
		std::cerr << "ERR " << __FILE__ << "." << __LINE__ << 
			": pokemon_nonvolatile stream has header of type \"" << lines.at(0).substr(0, header.size()) << 
			"\" (needs to be \"" << header <<
			"\") and is incompatible with this program!\n";

		return false;
	}

	std::vector<std::string> tokens = INI::tokenize(lines.at(iLine), "\t");
		
	if (!INI::checkRangeB(tokens.size(), (size_t)24, (size_t)24)) { return false; }
		
	//teammate nickname
	size_t iToken = 1;
	setName(tokens.at(iToken).substr(0, 20));

	// find species index:
	iToken = 2;
	{
		const pokemon_base* cSpecies = orphanCheck_ptr(pkdex->getPokemon(), mismatchedPokemon, tokens.at(iToken));
		if (cSpecies == NULL) 
		{ } //orphan!
		else 
		{ setBase(*cSpecies); }
	}
		
	// teammate level
	iToken = 3;
	{
		uint32_t _level;
		if (!INI::setArgAndPrintError("pokemon_nonvolatile level", tokens.at(iToken), _level, iLine, iToken)) { return false; }
		if (!INI::checkRangeB(_level, (uint32_t)1, (uint32_t)100)) { return false; }
		setLevel(_level);
	}

	// find item index:
	iToken = 4;
	{
		const item* cItem = NULL;

		if (tokens.at(iToken).compare("NONE") != 0)
		{
			cItem = orphanCheck_ptr(pkdex->getItems(), mismatchedItems, tokens.at(iToken));
		}
			
		if (cItem == NULL) { } // orphan!
		else if (!cItem->isImplemented())
		{
			if (mismatchedItems != NULL) {orphanAddToVector(*mismatchedItems, tokens.at(iToken)); }
			if (verbose >= 5)
			{
				std::cerr << "WAR " << __FILE__ << "." << __LINE__ << 
					": pokemon \"" << getBase().getName() <<
					"\" cannot use unimplemented item \"" << cItem->getName() <<
					"\"!\n";
			}
			cItem = NULL;
		}

		if (cItem == NULL)
		{
			setNoInitialItem();
		}
		else
		{
			setInitialItem(*cItem); 
		}
	}
		
	// teammate gender
	iToken = 5;
	{
		uint32_t _sex;
		if (!INI::setArgAndPrintError("pokemon_nonvolatile sex", tokens.at(iToken), _sex, iLine, iToken)) { return false; }
		if (!INI::checkRangeB(_sex, (uint32_t)0, (uint32_t)2)) { return false; }
		setSex(_sex);
	}

	// find ability index:
	iToken = 6;
	{
		const ability* cAbility = NULL;

		if (pokemonExists())
		{
			if (tokens.at(iToken).compare("NONE") != 0)
			{
				cAbility = orphan::orphanCheck_ptr(pkdex->getAbilities(), mismatchedAbilities, tokens.at(iToken));
			}

			const pokemon_base& cBase = getBase();
			if (cAbility == NULL) { } // orphan!
			else if (!std::binary_search(cBase.abilities.begin(), cBase.abilities.end(), cAbility))
			{
				if (mismatchedAbilities != NULL) { orphanAddToVector(*mismatchedAbilities, tokens.at(iToken)); }
				if (verbose >= 5)
				{
					std::cerr << "WAR " << __FILE__ << "." << __LINE__ << 
						": pokemon \"" << getBase().getName() <<
						"\" does not own ability \"" << cAbility->getName() <<
						"\"!\n";
				}
				cAbility = NULL;
			}
			else if (!cAbility->isImplemented())
			{
				if (mismatchedAbilities != NULL) { orphanAddToVector(*mismatchedAbilities, tokens.at(iToken)); }
				if (verbose >= 5)
				{
					std::cerr << "WAR " << __FILE__ << "." << __LINE__ << 
						": pokemon \"" << getBase().getName() <<
						"\" cannot use unimplemented ability \"" << cAbility->getName() <<
						"\"!\n";
				}
				cAbility = NULL;
			}
		} // endOf if pokemon exists
		if (cAbility != NULL)
		{
			setAbility(*cAbility);
		}
		else
		{
			setNoAbility();
		}
	}

	// find nature index:
	iToken = 7;
	{
		const nature* cNature = NULL;

		if (tokens.at(iToken).compare("NONE") != 0)
		{
			cNature = orphanCheck_ptr(pkdex->getNatures(), mismatchedNatures, tokens.at(iToken));
		}

		if (cNature != NULL)
		{
			setNature(*cNature);
		}
		else
		{
			setNoNature();
		}
	}

	// find move indecies:
	iToken = 8;
	for (size_t iAction = 0; iAction < 4; iAction++)
	{
		if (tokens.at(iToken + iAction).compare("NONE") == 0)
		{
			continue;
		}
		const move* cMove = orphanCheck_ptr(pkdex->getMoves(), mismatchedMoves, tokens.at(iToken + iAction));
		if (cMove == NULL) 
		{ 
			if (verbose >= 5)
			{
				// printed out later
				std::cerr << "WAR " << __FILE__ << "." << __LINE__ << 
					": pokemon \"" << getBase().getName() <<
					"\" has nonexistent move at index " << iAction <<
					":\"" << tokens.at(iToken + iAction) <<
					"\"!\n";
			}
			continue;
		} //orphan!
#ifndef _ALLOWINVALIDTEAMS
		if (pokemonExists())
		{
			if (!isLegalAdd(*cMove)) // TODO: warning, asserts pokemonExists
			{
				if (mismatchedMoves != NULL) { orphanAddToVector(*mismatchedMoves, tokens.at(iToken + iAction)); }
				if (verbose >= 5)
				{
					std::cerr << "WAR " << __FILE__ << "." << __LINE__ << 
						": pokemon \"" << getBase().getName() <<
						"\" cannot use " << (!cMove->isImplemented()?"unimplemented":"illegal") <<
						" move at index " << iAction <<
						":\"" << tokens.at(iToken + iAction) <<
						"\"!\n";
				}
				continue;
			}
		}
#endif
		// add move, only if all preconditions for its being added have been met:
		{
			addMove(move_nonvolatile(*cMove)); 
		}
	} //end of move indecies

#ifndef _ALLOWINVALIDTEAMS
	if (getNumMoves() == 0)
	{
		std::cerr << "ERR " << __FILE__ << "." << __LINE__ << 
			": pokemon \"" << getName() <<
			"\" - \"" << getBase().getName() <<
			" does not have enough valid moves (" << getNumMoves() <<
			")!\n";
		return false;
	}
#endif
		
	// input IV's and EV's:
	iToken = 12;
	unsigned int evAccumulator = 0;
	for (size_t ieValueIndex = 0; ieValueIndex < 6; ieValueIndex++)
	{
		uint32_t temp_IVEV;
		if (!INI::setArgAndPrintError("pokemon_nonvolatile IV", tokens.at(iToken + ieValueIndex), temp_IVEV, iLine, iToken + ieValueIndex)) { return false; }
		if (!INI::checkRangeB(temp_IVEV, (uint32_t)0, (uint32_t)31)) { return false; } 
		setIV(ieValueIndex, temp_IVEV);

		if (!INI::setArgAndPrintError("pokemon_nonvolatile EV", tokens.at(iToken + 6 + ieValueIndex), temp_IVEV, iLine, iToken + 6 + ieValueIndex)) { return false; }
		if (!INI::checkRangeB(temp_IVEV, (uint32_t)0, (uint32_t)255)) { return false; } 
		setEV(ieValueIndex, temp_IVEV);
			
		// if we haven't reached the max value yet:
		if (evAccumulator == UINT_MAX) { continue; }
			
		evAccumulator += temp_IVEV;
		if (evAccumulator > MAXEFFORTVALUE)
		{
			evAccumulator = UINT_MAX;
			std::cerr << "ERR " << __FILE__ << "." << __LINE__ << 
				": pokemon \"" << getName() <<
				"\" - \"" << getBase().getName() <<
				"\" has greater than maximum EV count of 510!\n";
			return false;
		}
	} // end of IV / EV

	iLine++;
	return pokemonExists();
} // endOf inputPokemon
