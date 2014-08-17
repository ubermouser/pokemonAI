//#define PKAI_IMPORT
#include "../inc/pkCU.h"

#include "../inc/fp_compare.h"

#include "../inc/planner_minimax_thread.h"

#include "../inc/vertex.h"
#include "../inc/agentMove.h"
#include "../inc/ply.h"

#include "../inc/otherMove.h"

otherMove* otherMove::dummyRoot = NULL;

otherMove::otherMove(agentMove& _parent, minimax_threadArg& _planner, unsigned int otherAction)
	:
#ifndef _DISABLEFINEGRAINEDLOCKING
	lock(),
#endif
	children(), 
	possibleEnvironments(),
	parent(_parent), 
	lbFitness(0), 
	probability(0),
	action(otherAction), 
	status(NODET_NOTEVAL)
{
};





void otherMove::pushbackChildren_nonLocking(minimax_threadArg& _planner, size_t numUnique, std::vector<ply*>& result)
{
	size_t iSize = possibleEnvironments.size();

	// determine minimum and maximum size of children
	fpType minProbability = std::numeric_limits<fpType>::max();
	fpType maxProbability = -std::numeric_limits<fpType>::max();
	for (size_t iEnv = 0; iEnv != iSize; iEnv++)
	{
		const environment_possible& cEnvironment = possibleEnvironments[iEnv];
		fpType cProbability = cEnvironment.getProbability().to_double();

		if (cProbability > maxProbability) { maxProbability = cProbability; }
		if (cProbability < minProbability) { minProbability = cProbability; }
	}

	// accumulators for a preemptive evaluation of this node
	fpType nlbFitness = 0;
	fpType nProbability = 0;

	// reserve space for children to be added
	children.reserve(children.size() + numUnique);
	result.reserve(result.size() + numUnique);

	for (size_t iEnv = 0; iEnv != iSize; ++iEnv)
	{
		const environment_possible& cEnvironment = possibleEnvironments[iEnv];
		fpType cProbability = cEnvironment.getProbability().to_double();

		// don't add pruned environments
		if (cEnvironment.isPruned()){ continue; }
		assert(iEnv < UINT16_MAX);

		ply* currentPly = new ply(cEnvironment, 
			_planner, 
			getParent(), // ply
			parent, // agent
			*this, //other
			(uint16_t) iEnv, // #
			minProbability,
			maxProbability);

		children.push_back(currentPly);
		result.push_back(currentPly);

		if (mostlyGT(currentPly->probability, 0.0))
		{
			nlbFitness += currentPly->lbFitness * cProbability;
			nProbability += currentPly->probability * cProbability;
		}
	} //endOf foreach environment

	assert(children.size() == numUnique);
	assert(mostlyLTE(nlbFitness, nProbability) && mostlyGTE(nlbFitness, 0.0)); // check lowerbound fitness
	assert(mostlyLTE(nProbability, 1.0) && mostlyGTE(nProbability, nlbFitness)); // check probability (upperbound)

	// if the tentative evaluation is enough to prune the set, do not evaluate:
	if (isPruned(nlbFitness, nProbability))
	{
		for (unsigned int iChild = 0; iChild != numUnique; ++iChild)
		{
			ply& cPly = *children[iChild];

			cPly.lastAction = (uint8_t) cPly.order.size() + 1; // prevent this node from having children
			//cPly->priority = 0; // nodes that are "fully" evaluated are computed first
			cPly.priority = cPly.getPriority( // TODO: can we just replace the status portion of this instead of recomputing the entire priority code?
					cPly.depth, 
					getParent().action, 
					parent.action, 
					action, 
					cPly.status,
					minProbability, 
					maxProbability, 
					cPly.envP.getProbability().to_double());

			// set plys with uninitialized fitnesses to something sane
			if (cPly.lbFitness < 0.0)
			{
				cPly.lbFitness = 0.0;
				cPly.probability = 0.0;
			}
		}

		//probability = nProbability;
		//lbFitness = nlbFitness;
	}
}






void otherMove::pushbackChildren(minimax_threadArg& _planner, size_t numUnique, std::vector<ply*>& result)
{
#ifndef _DISABLEFINEGRAINEDLOCKING
	// lock this othermove
	boost::unique_lock<boost::mutex> _lock(lock);
#endif

	pushbackChildren_nonLocking(_planner, numUnique, result);
}





bool otherMove::generateChildren_noExistingCheck(minimax_threadArg& _planner, std::vector<ply*>& _children)
{
	// are all of the other team's potential moves pruned by alpha beta pruning?
	if (isPruned()) 
	{ 
#ifndef NDEBUG
		if (verbose >= 7)
		{
			fpType ubFitness = lbFitness + (1.0 - probability);

			boost::unique_lock<boost::mutex> lock(_planner.getStdioLock());
			std::cout << "PR-O" <<
				" d" << std::setw(2) << std::dec << (unsigned int) getParent().getDepth()*3 - 2 <<
				" i" << std::setw(2) << (unsigned int) getParent().action <<
				" a" << std::setw(2) << (unsigned int) parent.action <<
				" o" << std::setw(2) << (unsigned int) action <<
				" #  x" <<
				" lbFit" << std::setw(10) << lbFitness <<
				" ubFit" << std::setw(10) << ubFitness <<
				" cProb" << std::setw(10) << probability <<
				" fail " << ((mostlyGTE(lbFitness, parent.betaBound))?"low":((mostlyLTE(ubFitness, parent.parent.alphaBound))?"high":"other")) <<
				"\n";
			std::cout.flush();
		}
#endif
		// immediately prune this mode upon fitness propagation
		return false; 
	}

	// currently no support for adding to an existing possibleEnvironment vector
	assert(possibleEnvironments.empty());
	// this otherMove has not been pruned:
	assert(status == NODET_NOTEVAL);

	// this may possibly return 0 states if an error occurs. Just continue until
	// it doesn't return 0 states
	size_t numUnique = _planner.getCU().updateState(
		getParent().envP.getEnv(), 
		possibleEnvironments, 
		((_planner.getAgentTeam()==TEAM_A)?parent.action:action), 
		((_planner.getAgentTeam()==TEAM_A)?action:parent.action)
		);
		
	if (numUnique == 0) { return false; }

#ifndef NDEBUG
	if (verbose >= 7)
	{
		boost::unique_lock<boost::mutex> lock(_planner.getStdioLock());
		std::cout << "GEN-" <<
			" d" << std::setw(2) << std::dec << (unsigned int) getParent().getDepth()*3 - 2 <<
			" i" << std::setw(2) << (unsigned int) getParent().action <<
			" a" << std::setw(2) << (unsigned int) parent.action <<
			" o" << std::setw(2) << (unsigned int) action <<
			" #" << std::setw(3) << numUnique <<
			" 0x" << std::setfill('0') << std::setw(16) << std::hex << getParent().envP.getHash() << std::setfill(' ') <<
			" !#" << std::setw(3) << std::dec << possibleEnvironments.size() <<
			" c" << std::setw(3) << children.size() <<
			" q" << std::setw(3) << _planner.plyQueue.size() <<
			" cProb" << std::setw(10) << getParent().envP.getProbability().to_double() <<
			"\n";
		std::cout.flush();
	}
#endif

	pushbackChildren_nonLocking(_planner, numUnique, _children);

	return true;
}





ply& otherMove::getParent() const
{
	return parent.parent;
}





ply& otherMove::propagateFitness(const ply& child, minimax_threadArg& _planner, std::vector<ply*>& _children)
{
	ply* result;
	bool propagated = false;
	{
#ifndef _DISABLEFINEGRAINEDLOCKING
		boost::unique_lock<boost::mutex> _lock(lock);
#endif

		assert(!(child.status == NODET_NOTEVAL && mostlyGT(child.probability, 0.0))); // if a value is propagated (not pruned), it has an action that represented it

#ifndef NDEBUG
		fpType olbFitness = lbFitness;
		fpType oubFitness = olbFitness + (1.0 - probability);
#endif
		{
			fpType cProbability = child.envP.getProbability().to_double();
			lbFitness += child.lbFitness * cProbability; // it's assumed the caller has this locked already
			probability += child.probability * cProbability; // probability of the caller's node
		}

		if (status == NODET_NOTEVAL)
		{
			// set to either NODET_CCUTOFF or NODET_CFULLEVAL
			assert(child.status != NODET_NOTEVAL);
			status = (child.status & 1) + 2;
		}
		else if ((child.status & 1) == NODET_CUTOFF)
		{
			status = NODET_CCUTOFF;
		}

		assert(mostlyGTE(child.probability - child.lbFitness, 0.0)); // check upperbound fitness
		assert(mostlyLTE(lbFitness, probability) && mostlyGTE(lbFitness, 0.0)); // check lowerbound fitness
		assert(mostlyLTE(probability, 1.0) && mostlyGTE(probability, lbFitness)); // check probability (upperbound)

#ifndef NDEBUG
		fpType ubFitness = lbFitness + (1.0 - probability);

		if (child.probability > 0 && verbose >= ((child.isLeaf())?8:7))
		{
			boost::unique_lock<boost::mutex> lock(_planner.getStdioLock());

			std::cout << ((child.isLeaf())?"EVAL":"BP-P") <<
				" d" << std::setw(2) << std::dec << (unsigned int) parent.parent.getDepth()*3 - 3 <<
				" i" << std::setw(2) << (unsigned int) parent.parent.action <<
				" a" << std::setw(2) << (unsigned int) parent.action <<
				" o" << std::setw(2) << (unsigned int) action <<
				" #" << std::setw(3) << (unsigned int) child.action <<
				" 0x" << std::setfill('0') << std::setw(16) << std::hex << child.envP.getHash() << std::setfill(' ') <<
				" lbFit" << std::setw(10) << std::dec << lbFitness <<
				" ubFit" << std::setw(10) << ubFitness <<
				" lpFit" << std::setw(10) << child.lbFitness <<
				"\n";
			std::cout.flush();
		}
		else if (verbose >= ((child.isLeaf())?8:7))
		{
			boost::unique_lock<boost::mutex> lock(_planner.getStdioLock());

			std::cout << ((child.isLeaf())?"PRUN":"NB-P") <<
				" d" << std::setw(2) << std::dec << (unsigned int) parent.parent.getDepth()*3 - 3 <<
				" i" << std::setw(2) << (unsigned int) parent.parent.action <<
				" a" << std::setw(2) << (unsigned int) parent.action <<
				" o" << std::setw(2) << (unsigned int) action <<
				" #" << std::setw(3) << (unsigned int) child.action <<
				" 0x" << std::setfill('0') << std::setw(16) << std::hex << child.envP.getHash() << std::setfill(' ') <<
				" lbFit" << std::setw(10) << std::dec << lbFitness <<
				" ubFit" << std::setw(10) << ubFitness <<
				" fail " << ((mostlyGTE(lbFitness, parent.betaBound))?"low":((mostlyLTE(ubFitness, parent.parent.alphaBound))?"high":"other")) <<
				"\n";
			std::cout.flush();
		}
		// are the fitness bounds converging onto a single value as the number of evaluated nodes increases?
		assert(mostlyLTE(olbFitness, lbFitness) && mostlyGTE(oubFitness, ubFitness)); // assert that the bounds actually shrank
#endif

		result = &recurse(child, _planner, _children, propagated);
	} // implicitly unlock mutex

	if (propagated) { delete this; }
	return *result;
}





ply& otherMove::recurse(const ply& child, minimax_threadArg& _planner, std::vector<ply*>& _children, bool& propagated)
{
	ply* result = &getParent();

	// delete child from array
	size_t location;
	findChild(children, child.action, location); // MUST return true
	children.erase(children.begin() + location);

	// children of of agent-other move combinations aren't generated on the fly, so it's safe to delete when this reaches 0
	if (children.empty())
	{
		propagated = true;

		// sanitize lbFitness and probability
		if (mostlyEQ(lbFitness, 0.0))
		{
			lbFitness = 0.0;
		}
		else if (mostlyEQ(lbFitness, 1.0))
		{
			lbFitness = 1.0;
			probability = 1.0;
		}

		if (mostlyEQ(probability, 1.0))
		{
			probability = 1.0;
		}
		else if (mostlyEQ(probability, 0.0))
		{
			lbFitness = 0.0;
			probability = 0.0;
		}

		result = &parent.propagateFitness(*this, _planner, _children); // continue to recurse
		
	}

	return *result;
}





void otherMove::deleteTree_recursive()
{
	for (size_t iChild = 0; iChild < children.size(); iChild++)
	{
		children.at(iChild)->deleteTree_recursive();
	}
	children.clear();

	delete this;
}





bool otherMove::isPruned(fpType clbFitness, fpType cProbability) const
{
	// is the fitness of this otherMove something the other player would never evaluate?
	// Do not produce siblings of this node if this is the case

#ifndef NDEBUG
	fpType olbFitness = lbFitness;
	fpType oubFitness = olbFitness + (1.0 - probability);
#endif

	fpType nlbFitness = lbFitness + clbFitness;
	fpType nubFitness = nlbFitness + (1.0 - probability - cProbability);

	assert(mostlyLTE(lbFitness, nubFitness) && mostlyGTE(lbFitness, 0.0)); // check lowerbound fitness
	assert(mostlyLTE(nubFitness, 1.0) && mostlyGTE(nubFitness, nlbFitness)); // check upperbound fitness
	assert(mostlyLTE(olbFitness, nlbFitness) && mostlyGTE(oubFitness, nubFitness)); // assert that the bounds actually shrank

	// is it impossible for any other move to have greater (upper bound) fitness than the current other move?
	if (mostlyLTE(nubFitness, 0.0))
	{
		return true;
	}
	// is it impossible for any agent move to have greater (lower bound) fitness than the current agent move?
	else if (mostlyGTE(nlbFitness, 1.0))
	{
		return true;
	}
	// if it is impossible for any fully evaluated fitness to fit within the bounds
	else if (mostlyGT(parent.parent.alphaBound, parent.betaBound))
	{
		return true;
	}
	// if it is not possible for the current otherMove to be "worse" than the "worst" otherMove, and thus the other player would never choose this action:
	/*else if ((mostlyGTE(parent->probability, 1.0) && mostlyGTE(nlbFitness, parent->betaBound)) ||
		mostlyGT(nlbFitness, parent->betaBound))*/
	else if (mostlyGTE(nlbFitness, parent.betaBound))
	{
		return true;
	}
	// if it is not possible for the current agentMove to be "better" than the "best" agentMove, and thus the agent player would never choose this action:
	/*else if ((mostlyGTE(parent->probability, 1.0) && mostlyLTE(nubFitness, parent->parent->alphaBound)) ||
		mostlyLT(nubFitness, parent->parent->alphaBound))*/
	else if (mostlyLTE(nubFitness, parent.parent.alphaBound))
	{
		return true;
	}
	else
	{
		// recurse
		//return parent->isPruned();

		// no need to recurse, otherMove handles pruning at all levels
		return false;
	}
}

