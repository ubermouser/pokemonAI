/* 
 * File:   planners.h
 * Author: drendleman
 *
 * Created on September 13, 2020, 2:52 PM
 */

#ifndef PLANNERS_H
#define PLANNERS_H

#include <memory>
#include <string>

#include "planner.h"

namespace planners {

std::shared_ptr<Planner::Config> config(const std::string& type);
std::shared_ptr<Planner> choose(const std::string& _type, const Planner::Config& cfg);

};


#endif /* PLANNERS_H */

