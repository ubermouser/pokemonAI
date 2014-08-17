
#ifndef MOVE_VOLATILE_H
#define MOVE_VOLATILE_H

#include "../inc/pkai.h"

#include <stdint.h>

class move_nonvolatile;
class move;

union PKAISHARED move_volatile
{
	uint8_t raw;
	struct 
	{
	uint8_t status_nonvolatile : 1;
	uint8_t PPcurrent : 7;
	} data;

	/* Compares values of selected move. Base values are compared by
	 * pointer, volatile values are compared by value
	 * DEPRECIATED: hash and compare environment_volatile instead! */
	bool operator==(const move_volatile& other) const;
	bool operator!=(const move_volatile& other) const;

	/* resets values of PPcurrent and PPmax */
	void initialize(const move_nonvolatile& cMove);

	/* returns count of this move's PP */
	uint32_t getPP() const { return data.PPcurrent; };
	
	/* modifies this move's PP by value */
	bool modPP(const move_nonvolatile& cMove, int32_t value);
	
	/* can a pokemon use this move during its next turn? */
	bool hasPP() const;
};

#endif	/* MOVE_VOLATILE_H */

