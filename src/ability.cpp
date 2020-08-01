
//#define PKAI_EXPORT
#include "../inc/ability.h"
//#undef PKAI_EXPORT

const Ability* Ability::no_ability = NULL;

Ability::Ability() 
  : Name(), 
  Pluggable(),
  script()
{
}





Ability::Ability(const Ability& source) 
  : Name(source), 
  Pluggable(source),
  script(source.script)
{
}





Ability::~Ability() 
{
  script.clear();
}

