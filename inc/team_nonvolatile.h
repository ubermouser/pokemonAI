/* 
 * File:   team_nonvolatile.h
 * Author: Ubermouser
 *
 * Created on June 30, 2011, 6:52 PM
 */

#ifndef TEAM_NONVOLATILE_H
#define	TEAM_NONVOLATILE_H

#include "../inc/pkai.h"

#include <stdint.h>
#include <ostream>
#include <vector>
#include <boost/array.hpp>

#include "../inc/signature.h"

#include "../inc/pokemon_nonvolatile.h"
#include "../inc/name.h"

union team_volatile;

#define TEAM_NONVOLATILE_DIGESTSIZE (POKEMON_NONVOLATILE_DIGESTSIZE * 6 + 1)

class PKAISHARED team_nonvolatile : public name, public signature<team_nonvolatile, TEAM_NONVOLATILE_DIGESTSIZE> 
{

private:
	/* nonvolatile teammmates of this team */ 
	boost::array<pokemon_nonvolatile, 6> teammates;

	/* number of pokemon in this team */
	uint8_t numTeammates;
	
public:
	team_nonvolatile();
	team_nonvolatile(const team_nonvolatile& orig);
	~team_nonvolatile() { };

	pokemon_nonvolatile& teammate(size_t iTeammate);

	const pokemon_nonvolatile& getPKNV(const team_volatile& source) const;

	/* returns number of teammates current pokemon team has */
	size_t getNumTeammates() const
	{
		return (size_t) numTeammates;
	};

	static size_t getMaxNumTeammates()
	{
		return 6;
	}

	/* is this pokemon allowed to be on the given team according to the current ruleset? */
	bool isLegalAdd(const pokemon_nonvolatile& cPokemon) const;

	bool isLegalSet(size_t iPosition, const pokemon_nonvolatile& cBase) const;

	bool isLegalAdd(const pokemon_base& cPokemon) const;

	bool isLegalSet(size_t iPosition, const pokemon_base& cBase) const;

	/* add a pokemon to the array of pokemon in this team */
	void addPokemon(const pokemon_nonvolatile& cPokemon);

	/* remove a pokemon from the array of pokemon in this team */
	void removePokemon(size_t iPokemon);

	/* sets pokemon to swappedPokemon */
	void setPokemon(size_t iPokemon, const pokemon_nonvolatile& swappedPokemon);

	/* sets the lead pokemon. The pokemon that was the lead pokemon takes the index of the switched pokemon */
	void setLeadPokemon(size_t iPokemon);

	void initialize();

	void uninitialize();
	
	const pokemon_nonvolatile& teammate(size_t iTeammate) const;
	
	void output(std::ostream& oFile, bool printHeader = true) const;

	bool input(const std::vector<std::string>& lines, size_t& iLine);
	
	void createDigest_impl(boost::array<uint8_t, TEAM_NONVOLATILE_DIGESTSIZE>& digest) const;

	friend class pkIO;
	friend union team_volatile;

};

#endif	/* TEAM_NONVOLATILE_H */

