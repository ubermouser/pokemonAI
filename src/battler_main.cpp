#include <inc/pkai.h>

#include <memory>
#include <string>

#include <inc/game.h>
#include <inc/pkCU.h>
#include <inc/pokedex_static.h>

struct Config {
  PokedexStatic::Config pokedex;
  Game::Config game;
  PkCU::Config engine;

  std::string team_a;
  std::string team_b;
};

int main(int argc, char** argv) {
  verbose = 5;

  auto pokedex = PokedexStatic();
  auto engine = PkCU();

  auto team_a = TeamNonVolatile::loadFromFile("teams/hexTeamA.txt");
  auto team_b = TeamNonVolatile::loadFromFile("teams/hexTeamD.txt");

  auto game = Game().setTeam(0, team_a).setTeam(1, team_b).setVerbosity(3);

  game.rollout();
}

