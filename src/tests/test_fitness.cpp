#include <gtest/gtest.h>
#include "pokemonai/fitness.h"
#include "pokemonai/fp_compare.h"

TEST(FitnessTest, Uncertainty) {
  Fitness f1(fpType(0.5), FixType(0.7));
  EXPECT_TRUE(mostlyEQ(f1.uncertainty(), FixType(0.3)));
  
  Fitness f2(fpType(0.5), FixType(1.0));
  EXPECT_TRUE(mostlyEQ(f2.uncertainty(), FixType(0.0)));
}

TEST(FitnessTest, Bounds) {
  // Fitness range is 0 to 1 for generic Fitness type.
  // upperBound() = (value_ * certainty_) + (max_fitness() * uncertainty())
  // lowerBound() = (value_ * certainty_) + (min_fitness() * uncertainty())
  
  Fitness f1(fpType(0.5), FixType(0.8));
  // value_ = 0.5, certainty_ = 0.8, uncertainty = 0.2
  // max_fitness = 1.0, min_fitness = 0.0
  // upperBound = 0.5 * 0.8 + 1.0 * 0.2 = 0.4 + 0.2 = 0.6
  // lowerBound = 0.5 * 0.8 + 0.0 * 0.2 = 0.4 + 0.0 = 0.4
  
  EXPECT_TRUE(mostlyEQ(f1.upperBound(), fpType(0.6)));
  EXPECT_TRUE(mostlyEQ(f1.lowerBound(), fpType(0.4)));
  
  Fitness f2(fpType(0.7), FixType(1.0));
  EXPECT_TRUE(mostlyEQ(f2.upperBound(), fpType(0.7)));
  EXPECT_TRUE(mostlyEQ(f2.lowerBound(), fpType(0.7)));
}

TEST(FitnessTest, Expand) {
  Fitness f1(fpType(0.5), FixType(0.8));
  FixType prob(0.5);
  Fitness f2 = f1.expand(prob);
  
  EXPECT_TRUE(mostlyEQ(f2.value(), fpType(0.5)));
  EXPECT_TRUE(mostlyEQ(f2.certainty(), FixType(0.4))); // 0.8 * 0.5
}

TEST(FitnessTest, Comparison) {
    Fitness f1(fpType(0.5), FixType(0.8)); // Bounds [0.4, 0.6]
    Fitness f2(fpType(0.8), FixType(0.9)); // Bounds [0.72, 0.82]
    
    // operator < uses upperBound < rhs.lowerBound
    // 0.6 < 0.72 is true
    EXPECT_LT(f1, f2);
    // 0.72 > 0.6 is true
    EXPECT_GT(f2, f1);
    
    Fitness f3(fpType(0.5), FixType(1.0)); // Bounds [0.5, 0.5]
    // f1.upperBound (0.6) < f3.lowerBound (0.5) is false
    EXPECT_FALSE(f1 < f3);
    // f3.upperBound (0.5) < f1.lowerBound (0.4) is false
    EXPECT_FALSE(f3 < f1);
}

TEST(FitnessTest, Addition) {
    Fitness f1(fpType(0.4), FixType(0.5));
    Fitness f2(fpType(0.8), FixType(0.3));
    
    // f1 += f2
    // value_ = (c1 * v1 + c2 * v2) / (c1 + c2)
    // value_ = (0.5 * 0.4 + 0.3 * 0.8) / (0.5 + 0.3)
    // value_ = (0.2 + 0.24) / 0.8 = 0.44 / 0.8 = 0.55
    // certainty_ = c1 + c2 = 0.8
    
    f1 += f2;
    EXPECT_TRUE(mostlyEQ(f1.value(), fpType(0.55)));
    EXPECT_TRUE(mostlyEQ(f1.certainty(), FixType(0.8)));
}
