#ifndef BINARY_EMBEDDING_H
#define BINARY_EMBEDDING_H

#include <array>
#include <cstddef>
#include <cstdint>

/**
 * @brief Binary-hot embedding for a scalar value.
 * 
 * Encodes the input value as a "binary-hot" vector (bit decomposition).
 * 
 * @tparam L Number of bits in the embedding.
 */
template <std::size_t L>
class BinaryEmbedding : public std::array<float, L> {
public:
 BinaryEmbedding() = default;

 /**
  * @brief Construct a new Binary Embedding object.
  *
  * @param value The value to encode.
  */
 template <typename T>
 explicit BinaryEmbedding(T value) {
   static_assert(L > 0, "Number of bits L must be greater than 0");

   const uint64_t val = static_cast<uint64_t>(value);

   for (std::size_t i = 0; i < L; ++i) {
     (*this)[i] = (val & (1ULL << i)) ? 1.0f : 0.0f;
   }
 }
};

#endif /* BINARY_EMBEDDING_H */
