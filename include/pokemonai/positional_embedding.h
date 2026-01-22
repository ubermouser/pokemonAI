#ifndef POSITIONAL_EMBEDDING_H
#define POSITIONAL_EMBEDDING_H

#include <array>
#include <cmath>
#include <cstddef>

/**
 * @brief NeRF-style positional embedding for a scalar value.
 * 
 * Computes an embedding of a floating point value in the same manner as NeRF (https://arxiv.org/abs/2006.10739).
 * The embedding consists of L frequency bands, each with a sine and cosine component.
 * Frequencies are log-sampled between minFreq and maxFreq.
 * 
 * Formula: [sin(f_k * pi * x), cos(f_k * pi * x), ...] where f_k is log-sampled.
 * 
 * @tparam L Number of frequency bands.
 * @tparam MinFreqT Fixed-point numerator for minimum frequency.
 * @tparam MaxFreqT Fixed-point numerator for maximum frequency.
 * @tparam FreqD Denominator for frequencies.
 */
template <std::size_t L, int MinFreqT = 1, int MaxFreqT = 1024, int FreqD = 1>
class PositionalEmbedding : public std::array<float, L * 2> {
public:
    static constexpr double minFreq = static_cast<double>(MinFreqT) / FreqD;
    static constexpr double maxFreq = static_cast<double>(MaxFreqT) / FreqD;

    /**
     * @brief Construct a new Positional Embedding object.
     * 
     * @param value The floating point value to encode.
     */
    template <typename T>
    explicit PositionalEmbedding(T value) {
        static_assert(L > 0, "Number of frequency bands L must be greater than 0");
        static_assert(maxFreq > minFreq, "Maximum frequency must be greater than minimum frequency");
        
        const double x = static_cast<double>(value);
        const double pi = std::acos(-1.0);
        
        for (std::size_t i = 0; i < L; ++i) {
            const double frequency = getFrequency(i);
            const double arg = frequency * pi * x;
            
            (*this)[i * 2] = static_cast<float>(std::sin(arg));
            (*this)[i * 2 + 1] = static_cast<float>(std::cos(arg));
        }
    }

    /**
     * @brief Compute the frequency for a given band index.
     * 
     * @param i Band index (0 to L-1).
     * @return double The log-sampled frequency.
     */
    static double getFrequency(std::size_t i) {
        if constexpr (L == 1) {
            return minFreq;
        } else {
            return minFreq * std::pow(maxFreq / minFreq, static_cast<double>(i) / (L - 1));
        }
    }
};

#endif /* POSITIONAL_EMBEDDING_H */
