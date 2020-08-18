//#define PKAI_IMPORT
#include "../inc/planner.h"

#include <sstream>

const std::vector<PlannerResult> Planner::emptyResults;


bool Planner::isInitialized() const {
    if (agentTeam_ >= 2) { return false; }
    if (nv_ == NULL) { return false; }
    if (cu_ == NULL) { return false; }
    if (eval_ == NULL) { return false; }
    if (!eval_->isInitialized()) { return false; }

    return true;
}


void Planner::setEnvironment(std::shared_ptr<const EnvironmentNonvolatile>& nv) {
  nv_ = nv;
  if (cu_ != NULL) { cu_->setEnvironment(nv); }
  if (eval_ != NULL) { eval_->setEnvironment(nv); }
}


void Planner::setEngine(std::shared_ptr<PkCU>& cu) {
  cu_ = cu;
  if (nv_ != NULL) { cu_->setEnvironment(nv_); }
};


void Planner::setEvaluator(std::shared_ptr<Evaluator>& eval) {
  eval_ = eval;
  if (nv_ != NULL) { eval_->setEnvironment(nv_); }
  resetName();
}


void Planner::resetName() {
  std::ostringstream name;
  name << baseName();
  name << "-";
  name << ((eval_!= NULL) ? eval_->getName(): "NULLEVAL");
  setName(name.str());
}
