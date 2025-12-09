#include "pokemonai/serializable.h"

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>

namespace pt = boost::property_tree;


void Serializer::deserialize(const std::string& path, Serializer& deserializer) {
  pt::ptree deserialized;
  pt::read_json(path, deserialized);

  deserializer.input(deserialized);
}


void Serializer::serialize(const std::string& path, const Serializer& serializer, bool printHeader) {
  pt::ptree serialized = serializer.output(printHeader);

  pt::write_json(path, serialized, std::locale(), true);
}