#ifndef ACTOR_H
#define ACTOR_H

#include <assert.h>

#include <cstddef>
#include <cstdint>
#include <functional>
#include <iosfwd>

/**
 * @struct Actor
 * @brief describes the ordering of when active pokemon move
 */
class Actor {
 public:
  uint32_t iTeam_;      // team of the pokemon that moves next
  uint32_t iTeammate_;  // teammate index of pokemon that moves next

  Actor(size_t iTeam, size_t iTeammate) : iTeam_(iTeam), iTeammate_(iTeammate) {
    assert(iTeam < 2);
    assert(iTeammate < 6);
  }

  size_t iTeam() const { return iTeam_; }
  size_t iTeammate() const { return iTeammate_; }

  const uint64_t* data() const {
    return reinterpret_cast<const uint64_t*>(this);
  }
  uint64_t* data() { return reinterpret_cast<uint64_t*>(this); }

  bool operator==(const Actor& other) const;
  bool operator!=(const Actor& other) const { return !(*this == other); }

  void print() const;
  void print(std::ostream& os) const;
};

std::ostream& operator<<(std::ostream& os, const Actor& actor);

namespace std {
template <>
struct hash<Actor> {
  size_t operator()(const Actor& a) const;
};
}  // namespace std

#endif /* ACTOR_H */