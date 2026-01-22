#include <gtest/gtest.h>
#include "pokemonai/positional_embedding.h"
#include "pokemonai/binary_embedding.h"
#include <cmath>


TEST(PositionalEmbeddingTest, SingleBand) {
    // L=1, x=0, minFreq=1, sin(pi*0)=0, cos(pi*0)=1
    PositionalEmbedding<1, 1, 1024, 1> emb0(0.0);
    EXPECT_NEAR(emb0[0], 0.0f, 1e-6);
    EXPECT_NEAR(emb0[1], 1.0f, 1e-6);

    // L=1, x=0.5, minFreq=2, sin(2*pi*0.5)=sin(pi)=0, cos(pi)=-1
    PositionalEmbedding<1, 2, 1024, 1> emb1(0.5);
    EXPECT_NEAR(emb1[0], 0.0f, 1e-6);
    EXPECT_NEAR(emb1[1], -1.0f, 1e-6);
}


TEST(PositionalEmbeddingTest, MultipleBands) {
    // L=2, minFreq=1, maxFreq=2
    // f0 = 1: sin(pi*x), cos(pi*x)
    // f1 = 2: sin(2*pi*x), cos(2*pi*x)
    PositionalEmbedding<2, 1, 2, 1> emb(0.25);
    
    // f0, x=0.25: sin(pi/4) = 0.707
    EXPECT_NEAR(emb[0], std::sin(M_PI * 0.25), 1e-6);
    EXPECT_NEAR(emb[1], std::cos(M_PI * 0.25), 1e-6);
    
    // f1, x=0.25: sin(2*pi/4) = 1
    EXPECT_NEAR(emb[2], 1.0f, 1e-6);
    EXPECT_NEAR(emb[3], 0.0f, 1e-6);
}


TEST(PositionalEmbeddingTest, RangeCheck) {
    // Verify values are within [-1, 1] for various inputs
    for (double x = -10.0; x <= 10.0; x += 0.1) {
        PositionalEmbedding<10, 5, 5000, 10> emb(x);
        for (float val : emb) {
            EXPECT_GE(val, -1.0001f);
            EXPECT_LE(val, 1.0001f);
        }
    }
}


TEST(PositionalEmbeddingTest, LogSampling) {
    // L=3, minFreq=1, maxFreq=100
    // f0 = 1
    // f1 = 1 * (100/1)^(1/2) = 10
    // f2 = 1 * (100/1)^(2/2) = 100
    PositionalEmbedding<3, 1, 100, 1> emb(0.1);
    
    // f0=1, x=0.1: sin(0.1*pi), cos(0.1*pi)
    EXPECT_NEAR(emb[0], std::sin(1.0 * M_PI * 0.1), 1e-6);
    EXPECT_NEAR(emb[1], std::cos(1.0 * M_PI * 0.1), 1e-6);
    
    // f1=10, x=0.1: sin(10*0.1*pi) = sin(pi) = 0
    EXPECT_NEAR(emb[2], 0.0f, 1e-6);
    EXPECT_NEAR(emb[3], -1.0f, 1e-6);
    
    // f2=100, x=0.1: sin(100*0.1*pi) = sin(10*pi) = 0
    EXPECT_NEAR(emb[4], 0.0f, 1e-6);
    EXPECT_NEAR(emb[5], 1.0f, 1e-6);
}


TEST(PositionalEmbeddingTest, HelperMethods) {
    // Verify static helper works
    using PE = PositionalEmbedding<2, 1, 10, 1>;
    EXPECT_NEAR(PE::minFreq, 1.0, 1e-9);
    EXPECT_NEAR(PE::maxFreq, 10.0, 1e-9);
    EXPECT_NEAR(PE::getFrequency(0), 1.0, 1e-9);
    EXPECT_NEAR(PE::getFrequency(1), 10.0, 1e-9);
}


TEST(BinaryEmbeddingTest, BasicEncoding) {
    // 5 = 101 in binary
    // bit 0: 1, bit 1: 0, bit 2: 1
    BinaryEmbedding<4> emb5(5);
    EXPECT_FLOAT_EQ(emb5[0], 1.0f);
    EXPECT_FLOAT_EQ(emb5[1], 0.0f);
    EXPECT_FLOAT_EQ(emb5[2], 1.0f);
    EXPECT_FLOAT_EQ(emb5[3], 0.0f);

    // 10 = 1010 in binary
    // bit 0: 0, bit 1: 1, bit 2: 0, bit 3: 1
    BinaryEmbedding<4> emb10(10);
    EXPECT_FLOAT_EQ(emb10[0], 0.0f);
    EXPECT_FLOAT_EQ(emb10[1], 1.0f);
    EXPECT_FLOAT_EQ(emb10[2], 0.0f);
    EXPECT_FLOAT_EQ(emb10[3], 1.0f);
}


TEST(BinaryEmbeddingTest, LargeValue) {
    // 255 = 11111111
    BinaryEmbedding<8> emb255(255);
    for (size_t i = 0; i < 8; ++i) {
        EXPECT_FLOAT_EQ(emb255[i], 1.0f);
    }

    BinaryEmbedding<8> emb256(256);
    for (size_t i = 0; i < 8; ++i) {
        EXPECT_FLOAT_EQ(emb256[i], 0.0f);
    }
}


TEST(BinaryEmbeddingTest, FractionalValue) {
    // Static cast to uint64_t should truncate
    BinaryEmbedding<4> emb(7.9); // 7 = 111 binary
    EXPECT_FLOAT_EQ(emb[0], 1.0f);
    EXPECT_FLOAT_EQ(emb[1], 1.0f);
    EXPECT_FLOAT_EQ(emb[2], 1.0f);
    EXPECT_FLOAT_EQ(emb[3], 0.0f);
}
