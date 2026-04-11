#include <gtest/gtest.h>
#include "pokemonai/action.h"
#include <sstream>

class ActionTest : public ::testing::Test {};


TEST_F(ActionTest, StreamSwap) {
  Action result;
  std::istringstream input("S5");
  input >> result;
  EXPECT_EQ(result, Action::swap(4));
}


TEST_F(ActionTest, StreamMoveAlly) {
  Action result;
  std::istringstream input("m2-f4");
  input >> result;
  EXPECT_EQ(result, Action::moveAlly(1, 3));
}


TEST_F(ActionTest, StreamMoveEnemy) {
  Action result;
  std::istringstream input("m2-4");
  input >> result;
  EXPECT_EQ(result, Action::moveEnemy(1, 3));
}


TEST_F(ActionTest, StreamRoundTrip) {
  Action result;
  std::stringstream input;
  input << Action::moveAlly(1, 3);
  input >> result;
  EXPECT_EQ(result, Action::moveAlly(1, 3));
}


TEST_F(ActionTest, StreamStruggle) {
  Action result;
  std::istringstream input("ms");
  input >> result;
  EXPECT_EQ(result, Action::struggle());
}


TEST_F(ActionTest, StreamWait) {
  Action result;
  std::istringstream input("w");
  input >> result;
  EXPECT_EQ(result, Action::wait());
}


TEST_F(ActionTest, StreamMoveFriendlyAdjacent) {
  Action result;
  std::istringstream input("m2-fa");
  input >> result;
  EXPECT_EQ(result, Action::moveAdjacentAlly(1));
}


TEST_F(ActionTest, StreamMoveHostileAdjacent) {
  Action result;
  std::istringstream input("m3-a");
  input >> result;
  EXPECT_EQ(result, Action::moveAdjacentEnemy(2));
}


TEST_F(ActionTest, StreamMoveBothAdjacent) {
  Action result;
  std::istringstream input("m4-fa-a");
  input >> result;
  EXPECT_EQ(result, Action::moveAdjacent(3));
}


TEST_F(ActionTest, StreamGarbage) {
  Action result;
  std::istringstream input("garbage");
  input >> result;
  EXPECT_FALSE(input);
}
