
#ifndef ORPHAN_H
#define ORPHAN_H

#include <vector>
#include <string>
#include <algorithm>

namespace orphan
{
	static void orphanAddToVector(std::vector<std::string>& set, const std::string& _orphan)
	{
		std::string orphan(_orphan);
		std::transform(orphan.begin(), orphan.end(), orphan.begin(), ::tolower);
		size_t lower = 0;
		size_t upper = set.size();
		size_t pivot;
		int compareValue;
		if (set.size() > 0)
		{
			while (upper > lower)
			{
				pivot = (lower+upper)/2;
				{
					std::string oString(set.at(pivot));
					std::transform(oString.begin(), oString.end(), oString.begin(), ::tolower);
					compareValue = orphan.compare(oString);
				}
				if (compareValue == 0)
				{
					return; // duplicate exists, don't add
				}
				else if ( compareValue < 0 )
				{
					// if the orphan is less than the pivot,
					// upper bound is now the pivot-1.
					upper = pivot;
				}
				else
				{
					// if the orphan is greater than the pivot,
					// lower bound is now the pivot+1.
					lower = pivot+1;
				}
			}
		}
		//orphan wasn't found in the vector, add it in the correct sorted position
		set.insert(set.begin()+upper,_orphan);
	}; // end of orphanAddToVector

	template <class libraryType>
	static size_t orphanCheck(const std::vector<libraryType>& library, std::vector<std::string>* set, const std::string& _orphan)
	{
		std::string orphan(_orphan);
		std::transform(orphan.begin(), orphan.end(), orphan.begin(), ::tolower);
		size_t lower = 0;
		size_t upper = library.size();
		size_t pivot;
		int compareValue;
		if (library.size() > 0) // if the library size is zero, then everything we match against it will be an orphan
		{
			while (upper > lower)
			{
				pivot = (lower+upper)/2;
				{
					std::string oString(library.at(pivot).getName());
					std::transform(oString.begin(), oString.end(), oString.begin(), ::tolower);
					compareValue = orphan.compare(oString);
				}
				if ( compareValue == 0 ) 
				{
					return pivot; // this value is not an orphan
				}
				else if ( compareValue < 0 )
				{
					// if the potentialOrphan is less than the pivot,
					// upper bound is now the pivot-1.
					upper = pivot;
				}
				else
				{
					// if the potentialOrphan is greater than the pivot,
					// lower bound is now the pivot+1.
					lower = pivot+1;
				}
			}
		}

		// no match for potentialOrphan, potentialOrphan is an orphan. Add it to set
		if (set != NULL)
		{
			orphanAddToVector(*set, orphan);
		}
		return SIZE_MAX;
	}; // end of orphanCheck

	template<class ptrType>
	static const ptrType* orphanCheck_ptr(const std::vector<ptrType>& library, std::vector<std::string>* set, const std::string& name)
	{
		size_t iValue = orphan::orphanCheck(library, set, name);
		return (iValue==SIZE_MAX)?NULL:&library.at(iValue);
	};
};

#endif /* ORPHAN_H */