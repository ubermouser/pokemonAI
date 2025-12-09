#ifndef SIGNATURE_H
#define SIGNATURE_H

#include "pokemonai/pkai.h"

#include <stdint.h>
#include <array>
#include <assert.h>

namespace hashes
{
  extern uint64_t hash_fnv(const void* _data, uint32_t len);

  extern uint64_t hash_murmur2(const void* _data, uint32_t len);

  extern uint64_t hash_murmur3(const void* _data, uint32_t len);
};

template<class derived, size_t digestSize = 0>
class Signature
{
public:
  Signature() = default;
  virtual ~Signature() = default;

  void createDigest(std::array<uint8_t, digestSize>& digest) const
  {
    static_cast<const derived*>(this)->createDigest_impl(digest);
  };

  uint64_t hash() const
  {
    if (digestSize > 0)
    {
      std::array<uint8_t, digestSize> digest;

      createDigest(digest);

      return genHash((void*) digest.data(), sizeof(digest));
    }
    else
    {
      return genHash((void *)this, sizeof(derived));
    }
  };

  static uint64_t genHash(const void* data, size_t len)
  {
#if defined(_USEFNVHASH)
    return hashes::hash_fnv(data, len);
#elif defined(_USEMURMUR2)
    return hashes::hash_murmur2(data, len);
#else
    return hashes::hash_murmur3(data, len);
#endif
  };

  template<class unknownType, class signatureWidth> 
  static void pack(const unknownType& _data, std::array<signatureWidth, digestSize>& signature, size_t& iSignature)
  {
    // size in bytes of the data we're packing into signature
    size_t size = sizeof(unknownType) / sizeof(signatureWidth);

    assert((sizeof(unknownType) % sizeof(signatureWidth)) == 0);
    assert(signature.size() >= (size + iSignature));

    const signatureWidth* data = (const signatureWidth*) &_data;

    for (size_t iChunk = 0; iChunk != size; ++iChunk)
    {
      signature[iSignature + iChunk] = data[iChunk];
    }

    iSignature += size;
  };

};

#endif /* SIGNATURE_H */

