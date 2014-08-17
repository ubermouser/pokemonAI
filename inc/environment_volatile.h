/* 
 * File:   PKAI_environment_volatile.h
 * Author: Ubermouser
 *
 * Created on June 8, 2011, 3:08 PM
 */

#ifndef ENVIRONMENT_VOLATILE_H
#define	ENVIRONMENT_VOLATILE_H

#include "../inc/pkai.h"

#include <boost/array.hpp>
#include <stdint.h>

#include "../inc/team_volatile.h"

class environment_nonvolatile;

union PKAISHARED environment_volatile
{
	uint64_t raw[16];
	struct
	{
		team_volatile teams[2];
	} data;
	
	/* Compares values of selected environment. Base values are compared by
	 * pointer, volatile values are compared by value */
	bool operator==(const environment_volatile& other) const;
	bool operator!=(const environment_volatile& other) const;

	const team_volatile& getTeam(size_t movesFirst) const ;
	const team_volatile& getOtherTeam(size_t movesFirst) const;
	
	team_volatile& getTeam(size_t movesFirst);
	team_volatile& getOtherTeam(size_t movesFirst);

	static environment_volatile create(const environment_nonvolatile& envNV);
	
	void initialize(const environment_nonvolatile& envNV);
};

#endif	/* ENVIRONMENT_VOLATILE_H */

