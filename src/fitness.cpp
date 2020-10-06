#include "../inc/fitness.h"

#include <iostream>
#include <boost/format.hpp>


FITNESS_TEMPLATE
void FITNESS_IMPL::print() const { std::cout << *this << "\n"; }


FITNESS_TEMPLATE
std::ostream& FITNESS_IMPL::print(std::ostream& os) const {
  os << boost::format("F{v=%4.2f c=%4.2f}") % double(value_) % double(certainty_);
  return os;
}

std::ostream& operator <<(std::ostream& os, const Fitness& fitness) {
  return fitness.print(os);
}


FITNESS_TEMPLATE void FITNESS_IMPL::assertValidity() const {
  assert(value_ >= min_fitness() && value_ <= max_fitness());
  assert(certainty_ >= zero() && certainty_ <= one());
  assert(upperBound() <= max_fitness() && lowerBound() >= min_fitness());
}


template class FitnessType<fpType, 0, 1, 1>;
