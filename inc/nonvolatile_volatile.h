/* 
 * File:   nonvolatile_volatile.h
 * Author: ubermouser
 *
 * Created on August 14, 2020, 6:45 PM
 */

#ifndef NONVOLATILE_VOLATILE_H
#define NONVOLATILE_VOLATILE_H

template<typename NonVolatileType, typename VolatileType>
class NonvolatileVolatilePair {
public:
  using nonvolatile_t = NonVolatileType;
  using volatile_t = VolatileType;
  using base_t = NonvolatileVolatilePair<NonVolatileType, VolatileType>;

  NonVolatileType* nv_;
  VolatileType* data_;

  NonvolatileVolatilePair() = delete;
  NonvolatileVolatilePair(const NonvolatileVolatilePair& other) = default;
  NonvolatileVolatilePair& operator=(const NonvolatileVolatilePair& source) = default;
  NonvolatileVolatilePair(
      typename base_t::nonvolatile_t& nv,
      typename base_t::volatile_t& data
  ): nv_(&nv), data_(&data) {};

  NonVolatileType& nv() const { return *nv_; }
  VolatileType& data() const { return *data_; }
  //operator NonVolatileType&() const { return *nv_; };
  operator VolatileType&() { return *data_; };
};

#endif /* NONVOLATILE_VOLATILE_H */

