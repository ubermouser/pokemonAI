#ifndef PLUGIN_H
#define PLUGIN_H

#include "pkai.h"

#include <stdint.h>
#include <string>
#include <vector>
#include <boost/function.hpp>

#include "pluggable.h"

class Pokedex;

typedef boost::function<bool (const Pokedex&, std::vector<plugin>&)> regExtension_type;

#endif /* PLUGIN_H */
