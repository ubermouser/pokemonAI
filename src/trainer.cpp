#include "../inc/trainer.h"

#include <omp.h>

void Trainer::initialize() {
  
}

void Trainer::gauntlet(const std::shared_ptr<TrueSkillTeam>& tsTeam, LeagueHeat& league) const {
  
}


GameHeat Trainer::singleGame(
    const std::shared_ptr<TrueSkillTeam>& team_a,
    const std::shared_ptr<TrueSkillTeam>& team_b) const {
  auto& game = games.at(omp_get_thread_num());

  auto setTSTeam = [&](size_t iTeam, auto& tsTeam){
    game->setTeam(iTeam, tsTeam->team->team);
    game->setPlanner(iTeam, tsTeam->evaluator->eval);
  };

  setTSTeam(0, team_a);
  setTSTeam(1, team_b);
  HeatResult result = game->run();

  return GameHeat{team_a, team_b, result};
}