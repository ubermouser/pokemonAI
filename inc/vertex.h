#ifndef VERTEX_H
#define	VERTEX_H

#include "../inc/pkai.h"

#include <vector>

using std::size_t;

class vertex
{
private:
  //vertex() {};
  //vertex& operator=(const vertex& source) {};
  //vertex(const vertex& other) {};
public:
  //virtual ~vertex() {};

  virtual size_t getAction() const = 0;
  //virtual std::vector<vertex*>& getChildren() = 0;

  /* returns FALSE if no such child exists */
  //template <class libraryType> bool findChild(unsigned int otherAction, unsigned int& location);

  bool compareEQ(size_t otherAction) const
  {
    return getAction() == otherAction;
  };

  bool compareLT(size_t otherAction) const
  {
    return getAction() < otherAction;
  };

  bool compareGT(size_t otherAction) const
  {
    return getAction() > otherAction;
  };

  template <class libraryType>
  static bool findChild(const std::vector<libraryType*>& children, size_t iAction, size_t& location)
  {
    size_t upper = children.size();
    size_t lower;
    size_t pivot;

    if (children.size() > 0)
    {
      lower = 0;
      while (upper > lower)
      {
        pivot = (lower+upper)/2;
        if ( children.at(pivot)->compareEQ(iAction) )
        {
          // found value
          location = pivot;
          return true;
        }
        else if ( children.at(pivot)->compareGT(iAction) )
        {
          // if the match is greater than the pivot,
          // upper bound is now the pivot-1.
          upper = pivot;
        }
        else
        {
          // if the match is less than the pivot,
          // lower bound is now the pivot+1.
          lower = pivot+1;
        }
      }
    }
    // doesn't exist within the set, point to the location where it should be added

    location = upper;
    return false;
  }
};

#endif
