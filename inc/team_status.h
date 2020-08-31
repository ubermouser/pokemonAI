/* 
 * File:   team_status.h
 * Author: ubermouser
 *
 * Created on August 14, 2020, 7:12 PM
 */

#ifndef TEAM_STATUS_H
#define TEAM_STATUS_H

struct VolatileStatus
{
  // boost array
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
  uint32_t lockIn_action : 3; // includes uproar, outrage, charge, and recharge
  uint32_t lockIn_duration : 3;
  uint32_t defensiveCurl : 1;
  uint32_t imprison : 1;
  uint32_t infatuate : 1;
  // END OF THIRD WORD

  void reset();
};

struct NonVolatileStatus
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

struct TeamStatus {
  VolatileStatus cTeammate;
  NonVolatileStatus nonvolatile;
};

#endif /* TEAM_STATUS_H */

