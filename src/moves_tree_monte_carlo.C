#include <string>
#include <stdexcept>
#include <unistd.h>
#include <iostream>
#include <sstream>
#include <stdlib.h>
#include <algorithm>
#include <bits/stdc++.h>
#include <chess.h>

namespace SeaChess {

#ifdef GRAPH_SUPPORT
int master_move_id;
#endif

//#define DEBUG_MONTE_CARLO 1
//#define DEBUG_RANDOM_MOVES_GAME 1
//#define DEBUG_HIGH_MOVES 1
//#define DEBUG_FIXED_RANDOM_SEED 1
//#define DEBUG_BEST_MOVE 1

#define GAMES_BETWEEEN_TIMEOUT_CHECKS 1000
  
//***********************************************************************************************
// build up tree of moves; pick the best one. monte-carlo...
//***********************************************************************************************

int MovesTreeMonteCarlo::ChooseMove(Move *next_move, Board &game_board, Move *suggested_move) {
  std::cout << "#  ChooseMove entered, turn " << NumberOfTurns() << "..." << std::endl;
  
#ifdef GRAPH_SUPPORT
  master_move_id = 0;
#endif

#ifdef DEBUG_FIXED_RANDOM_SEED
  // for predictability during debug, fix random seed...
  srand(1);
  std::cout << "#  Initial random #: " << rand() << std::endl;
#endif

  ResetTotalGamesCount();  
  ResetRandomGameStats();
  SetLevels(0); // starting level is NOT same as number of turns
  SetMaxLevels(75);
  SetMaxRandomGameLevels(1000);

  // evaluate moves 'til some threshhold reached...

  MovesTreeNode root;

  rollout_index = 0;
  ResetLastLevelVisited();

  StartClock();

  while( !MaxGamesExceeded() && !Timeout(move_time) && !next_move->GameOver()) {
    for (int i = 0; (i < (GAMES_BETWEEEN_TIMEOUT_CHECKS / RolloutCount())) && !MaxGamesExceeded(); i++) {
        ChooseMoveInner(&root,game_board,Color());
        if (next_move->GameOver())
          break;
    }
  }

  std::cout << "#  Total # of games simulated: " << TotalGamesCount() << std::endl;
  std::cout << "#  Number of move 'look-aheads' (levels): " << LastLevelVisited()
	    << ", max-levels: " << MaxLevels() << std::endl;

  int num_draws, num_checkmates, num_max_levels_reached;
  RandomGameStats(num_draws, num_checkmates, num_max_levels_reached);

  std::cout << "#  Random game stats: # draws: " << num_draws
            << ", # checkmates: " << num_checkmates
            << ", # 'max-levels exceeded' draws: " << num_max_levels_reached << std::endl;

  //GraphMovesToFile("moves", next_move); //<---generally only useful when small # of moves possible

  // at this point, the list of possible moves (moves explored) attached to the 'next' move
  // represents the list of potential moves...

  PickBestMove(&root,game_board,suggested_move);

#ifdef DEBUG_MONTE_CARLO
  float highest_node_uct;
  HighScoreMove(highest_node_uct,root,root,true);
#endif

  return TotalGamesCount();
}

void MovesTreeMonteCarlo::ChooseMoveInner(MovesTreeNode *current_node, Board &current_board,int current_color) {
}
  
}
