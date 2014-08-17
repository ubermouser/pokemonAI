/* 
 * File:   ability.h
 * Author: Ubermouser
 *
 * Created on June 8, 2011, 3:45 PM
 */

#ifndef ABILITY_H
#define ABILITY_H

#include "../inc/pkai.h"

#include <string>

#include "../inc/name.h"
#include "../inc/pluggable.h"

class PKAISHARED ability: public name, public pluggable
{
public:
	static const ability* no_ability;

	ability();
	ability(const ability& source);
	~ability();

	friend class pokedex;
private:

	std::string script;
};

#endif	/* ABILITY_H */

