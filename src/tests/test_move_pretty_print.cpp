#include "engine_test.hpp"
#include "pokemonai/move_volatile.h"
#include <sstream>

class MovePrettyPrintTest : public Gen4EngineTest {};

TEST_F(MovePrettyPrintTest, MoveVolatileOperatorOutput) {
  const auto& moves = pokedex_->getMoves();
  // Ensure we have a move to test with
  ASSERT_TRUE(moves.count("tackle"));
  const Move& tackle = moves.at("tackle");
  
  MoveNonVolatile mNV(tackle);
  MoveVolatileData data;
  data.PPcurrent = 35;
  data.status_nonvolatile = 0;
  
  ConstMoveVolatile cMV(mNV, data);
  
  std::stringstream ss;
  ss << cMV;
  
  std::string output = ss.str();

  SCOPED_TRACE(output);

  // Example output: "tackle" [normal] Physical Pwr: 35 PP: 35/56
  EXPECT_TRUE(output.find("\"tackle\"") != std::string::npos);
  EXPECT_TRUE(output.find("normal") != std::string::npos);
  EXPECT_TRUE(output.find("Physical") != std::string::npos);
  EXPECT_TRUE(output.find("Pwr: 35") != std::string::npos);
  EXPECT_TRUE(output.find("PP: 35") != std::string::npos);
}
