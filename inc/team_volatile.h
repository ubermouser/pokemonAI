/* 
 * File:   PKAI_team_volatile.h
 * Author: Ubermouser
 *
 * Created on June 8, 2011, 3:04 PM
 */

#ifndef TEAM_VOLATILE_H
#define	TEAM_VOLATILE_H

#include "../inc/pkai.h"

#include <stdint.h>
#include <boost/array.hpp>

#include "../inc/pokemon_volatile.h"

class team_nonvolatile;
class pokemon_nonvolatile;

struct volatileStatus
{
  // boost array
  union
  {
    int32_t raw;
    struct
    {
      int32_t B_ATK : 4;
      int32_t B_SPA : 4;
      int32_t B_DEF : 4;
      int32_t B_SPD : 4;
      int32_t B_SPE : 4;
      int32_t B_ACC : 4;
      int32_t B_EVA : 4;
      int32_t B_CHT : 4;
    } data;
  } boosts;
  // END OF FIRST WORD
  // volatile status:
  uint32_t confused : 3;
  uint32_t disable_duration : 3;
  uint32_t disable_action : 2; 
  uint32_t healBlock : 3;
  uint32_t encore_action : 2;
  uint32_t encore_duration : 3;
  // bit 1.16
  uint32_t iLastAction : 4; // updated only when moves like encore/torment are in play on the other team
  uint32_t curse : 1;
  uint32_t flinch : 1;
  uint32_t focusEnergy : 1;
  uint32_t mudSport : 1;
  uint32_t perishSong : 2;
  uint32_t torment : 1;
  uint32_t trap : 3; // partial trap 0-5, full trap 7
  uint32_t lockOn : 1;
  uint32_t identify : 1;
  // END OF SECOND WORD
  uint32_t substitute : 8;
  uint32_t nightmare : 1;
  uint32_t leechSeed : 1;
  uint32_t waterSport : 1;
  // bit 2.11
  // values not transferrable via baton pass or any other method:
  uint32_t itemScratch : 4; // free bits for the current item to use
  uint32_t numRoundsInPlay : 4; // updated only when conditions like slow start are in play
  uint32_t toxicPoison_tier : 4; // updated only when conditions like toxic poison are in play
  uint32_t chargeMove : 3; // includes uproar, rampage, charge, and recharge
  uint32_t defensiveCurl : 1;
  uint32_t momentum : 3;
  uint32_t imprison : 1;
  uint32_t infatuate : 1;
  // END OF THIRD WORD
};

struct nonvolatileStatus
{
  // index of the current pokemon
  uint32_t iCPokemon : 3;
  // weather effects:
  uint32_t weather_duration : 3;
  uint32_t weather_type : 3;
  uint32_t room_duration : 3;
  uint32_t room_type : 2;
  uint32_t gravity : 3;
  // entry hazards:
  uint32_t spikes : 2;
  // bit 0.16
  uint32_t stealthRock : 1;
  uint32_t toxicSpikes : 2;
  // team protection:
  uint32_t lightScreen : 3;
  uint32_t reflect : 3;
  uint32_t safeguard : 3;
  uint32_t mist : 1;
};

union PKAISHARED team_volatile
{
  uint64_t raw[8];
  struct
  {
    /* storage for 6 pokemon. Unused pokemon are zeroed */
    pokemon_volatile teammates[6];

    union
    {
      uint64_t raw[2];
      struct
      {
        union
        {
          uint32_t raw[3];
          volatileStatus data;
        } cTeammate;
        union
        {
          uint32_t raw;
          nonvolatileStatus data;
        } nonvolatile;
      } data;
    } status;
  } data;

  pokemon_volatile& teammate(size_t iTeammate) 
  { 
    return data.teammates[iTeammate]; 
  };

  const pokemon_volatile& teammate(size_t iTeammate) const 
  { 
    return data.teammates[iTeammate]; 
  };

  void resetVolatile();

  volatileStatus& getVolatile() { return data.status.data.cTeammate.data; };
  const volatileStatus& getVolatile() const { return data.status.data.cTeammate.data; };

  nonvolatileStatus& getNonVolatile() { return data.status.data.nonvolatile.data; };
  const nonvolatileStatus& getNonVolatile() const { return data.status.data.nonvolatile.data; };

  /* Resets all pokemon in this team */
  void initialize(const team_nonvolatile& nv);

  /* Retrieves a pointer to the current pokemon active on this team */
  const pokemon_volatile& getPKV() const { return data.teammates[getICPKV()]; };
  
  /* Retrieves a pointer to the current pokemon active on this team */
  pokemon_volatile& getPKV() { return data.teammates[getICPKV()]; };

  /* gets current index of pokemon volatile on this team */
  size_t getICPKV() const { return data.status.data.nonvolatile.data.iCPokemon; };
  
  /* Swaps the currently active pokemon with the target pokemon */
  bool swapPokemon(size_t iAction, bool preserveVolatile = false);
  
  /* returns number of teammates on this team that are still alive */
  uint32_t numTeammatesAlive() const;

  int32_t cGetBoost(size_t type) const;

  void cSetBoost(size_t type, int32_t value);

  bool cModBoost(size_t type, int32_t amt);

  uint32_t cGetFV_boosted(const team_nonvolatile& tNV, size_t type, int32_t tempBoost = 0) const;
  uint32_t cGetFV_boosted(const pokemon_nonvolatile& tNV, size_t type, int32_t tempBoost = 0) const;

  fpType cGetAccuracy_boosted(size_t type, int32_t tempBoost = 0) const;

  bool cHasPP() const;

  /* returns TRUE if the pokemon has more than 0 hitpoints */
  bool cIsAlive() const;

  /* increment target's hp by quantity. */
  void cModHP(const pokemon_nonvolatile& nonvolatile, int32_t quantity);

  /* set target's hp to quantity. */
  void cSetHP(const pokemon_nonvolatile& nonvolatile, uint32_t amt);

  /* set target's hp to % quantity of total */
  void cSetPercentHP(const pokemon_nonvolatile& nonvolatile, fpType percent);
  
  /* increment target's HP by percent of total. */
  void cModPercentHP(const pokemon_nonvolatile& nonvolatile, fpType percent);
  
  /* Compares values of selected team. Base values are compared by
    * pointer, volatile values are compared by value */
  bool operator==(const team_volatile& other) const;
  bool operator!=(const team_volatile& other) const;
};

#endif	/* TEAM_VOLATILE_H */

