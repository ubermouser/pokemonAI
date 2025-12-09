/* 
 * File:   gen4_scripts.h
 * Author: ubermouser
 *
 * Created on August 13, 2020, 8:34 AM
 */

#ifndef GEN4_SCRIPTS_H
#define GEN4_SCRIPTS_H

#include "pokedex.h"
#include "plugin.h"

#include <vector>

bool registerExtensions(const Pokedex& pkAI, std::vector<plugin>& extensions);

#endif /* GEN4_SCRIPTS_H */

