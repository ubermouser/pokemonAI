/* 
 * File:   serializeable.h
 * Author: drendleman
 *
 * Created on October 1, 2020, 12:35 PM
 */

#ifndef SERIALIZEABLE_H
#define SERIALIZEABLE_H

#include <string>
#include <boost/property_tree/ptree_fwd.hpp>

class Serializer {
public:
  Serializer() = default;
  virtual ~Serializer() = default;

  virtual void input(const boost::property_tree::ptree& ptree) = 0;
  virtual boost::property_tree::ptree output(bool printHeader = true) const = 0;

  virtual void save(const std::string& path, bool printHeader=true) const {
    serialize(path, *this, printHeader);
  }
protected:
  static void deserialize(const std::string& path, Serializer& deserializer);

  static void serialize(const std::string& path, const Serializer& serializer, bool printHeader);
};

template<typename Derived>
class Serializable : public Serializer {
public:
  Serializable() = default;
  virtual ~Serializable() = default;

  static Derived load(const std::string& path) {
    Derived result;
    deserialize(path, result);
    
    return result;
  }
};

#endif /* SERIALIZEABLE_H */

