#include <string>
#include <stdexcept>
#include <unistd.h>
#include <iostream>
#include <sstream>
#include <stdlib.h>
#include <algorithm>
#include <bits/stdc++.h>

#include <chess.h>
#include <random_moves_game.h>

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
       float incr_white_wins, incr_black_wins; 
       ChooseMoveInner(&root,incr_white_wins,incr_black_wins,game_board,Color());
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

void MovesTreeMonteCarlo::ChooseMoveInner(MovesTreeNode *node, float &incr_white_wins, float &incr_black_wins, Board &current_board,int current_color) {
#ifdef DEBUG_MONTE_CARLO
    std::cout << "[EngineMonteCarlo::ChooseMoveInner] entered, color: " 
              << ChessUtils::ColorAsStr(current_color) << ", level: " 
              << Levels() << " # visits:" << node->NumberOfVisits()
              << ", previous move: (" << EncodeMove(current_board,*node) << ")"
              << " # possible-moves: " << node->PossibleMovesCount() << "..." << std::endl;
#endif

  UpdateLastLevel(Levels());

  // is it a draw?

  if (MaxLevelsReached()) {
#ifdef DEBUG_MONTE_CARLO
    std::cout << "[EngineMonteCarlo::ChooseMoveInner] max-levels reached. Game ends in a draw." << std::endl;
#endif
    node->ScoreWinsCount(incr_white_wins,incr_black_wins,current_color,false);
    node->SetOutcome(DRAW);
    BumpTotalGamesCount();
    return;
  }

  // add list of possible moves for this game state, if needed...

  if (node->PossibleMovesCount() == 0) {
    // populate possible moves list...
#ifdef DEBUG_MONTE_CARLO
    std::cout << "[EngineMonteCarlo::ChooseMoveInner] get list of all possible moves for this game state..." << std::endl;
#endif
    // add valid moves to node...

    bool in_check = GetMoves(node,current_board,current_color);
#ifdef DEBUG_MONTE_CARLO
    std::cout << "[EngineMonteCarlo::ChooseMoveInner] there are " << node->PossibleMovesCount() 
              << " possible moves for this game state..." << std::endl;
#endif
     // no valid moves? then assume draw or checkmate
    if (node->PossibleMovesCount() == 0) {
      int win_color = (current_color == WHITE) ? BLACK : WHITE;
      node->ScoreWinsCount(incr_white_wins,incr_black_wins,win_color,in_check);
      if (in_check)
        node->SetOutcome(win_color == Color() ? CHECKMATE : RESIGN);
      else
        node->SetOutcome(DRAW);
      BumpTotalGamesCount();
#ifdef DEBUG_MONTE_CARLO
      std::cout << "[EngineMonteCarlo::ChooseMoveInner] game ends in " << (in_check ? "CheckMate!" : "Draw!") << ", winning color: " 
                << ChessUtils::ColorAsStr(win_color) <<std::endl;
#endif
      return;
    }
  }

  // explore based on the most promising move (select move with highest UCT)...

  float highest_node_uct = 0.0;

  MovesTreeNode *next_move = HighScoreMove(highest_node_uct,node);

  // there is a high score, nes pa?
  assert(next_move != NULL);

  Board updated_board = MovesTree::MakeMove(current_board, next_move);

  if (next_move->NumberOfVisits() == 0) {
    // we haven't visited this node before, do rollout and return...
    int number_of_rollout_simulations = Rollout(next_move, updated_board, current_board, OtherColor(current_color));
    node->SetWinCounts( incr_white_wins, incr_black_wins, next_move->NumberOfWhiteWins(), next_move->NumberOfBlackWins() );
    node->IncrementVisitCount(number_of_rollout_simulations);
#ifdef DEBUG_MONTE_CARLO
    std::cout << "[EngineMonteCarlo::ChooseMoveInner] returns from leaf node 'addition', incr wins white/black: " 
              << next_move->IncrementalWinsCountWhite() << "/" << next_move->IncrementalWinsCountBlack() 
              << "..." << std::endl;
#endif
    return;
  }

  // descend game tree to next level...

#ifdef DEBUG_MONTE_CARLO
  std::cout << "  ChooseMoveInner descending, next level: " << Levels() << "..." << std::endl;
#endif

  incr_white_wins = incr_black_wins = 0.0;
  
  ChooseMoveInner(next_move, incr_white_wins, incr_black_wins, updated_board, OtherColor(current_color));

  node->IncreaseWinsCounts( incr_white_wins, incr_black_wins );
  node->IncrementVisitCount(RolloutCount());

#ifdef DEBUG_MONTE_CARLO
  std::cout << "[EngineMonteCarlo::ChooseMoveInner] returns from level " << Levels() << ", incr wins white/black: " 
            << incr_white_wins << "/" << incr_black_wins << "..." << std::endl;
#endif

  PreviousLevel();
}

//***********************************************************************************************
// play N random games starting at 'node'. rollout has been called on a node that has not
// been visited before...
//***********************************************************************************************

int MovesTreeMonteCarlo::Rollout(MovesTreeNode *current_node, Board &current_board, Board &previous_board, int current_color) {
#ifdef DEBUG_MONTE_CARLO
  std::cout << "[EngineMonteCarlo::rollout] entered for color " << ChessUtils::ColorAsStr(current_color) << "..." << std::endl;
#endif

  assert(current_node->PossibleMovesCount() == 0); // this node hasn't been visited yet, nes pa?

  // play N games from this node (move) to yield an aggregate score...

  bool in_check = false;

  for (auto i = 0; i < RolloutCount(); i++) {
     SeaChess::RandomMovesGame rndgame(MaxRandomGameLevels(),NumberOfTurns());
     float white_score,black_score;
     rndgame.Play(white_score,black_score,current_board,current_color,Levels());
#ifdef DEBUG_MONTE_CARLO
     std::cout << "[EngineMonteCarlo::rollout] random game wh/bl wins: " << white_score << "/" << black_score << std::endl;
#endif
     current_node->IncreaseWinsCounts( white_score, black_score );
     current_node->IncrementVisitCount();
     BumpTotalGamesCount();
     int num_draws, num_checkmates, num_max_levels;
     rndgame.RandomGameStats(num_draws, num_checkmates, num_max_levels); 
     UpdateRandomGameStats(num_draws, num_checkmates, num_max_levels); 
#ifdef DEBUG_MONTE_CARLO
     std::cout << "[EngineMonteCarlo::rollout] current node wh/bl wins: " << current_node->IncrementalWinsCountWhite()
               << "/" << current_node->IncrementalWinsCountBlack() << std::endl;
#endif
  }

#ifdef DEBUG_MONTE_CARLO
  std::cout << "[EngineMonteCarlo::rollout] exited, moves count: " << current_node->NumberOfVisits() 
            << ", #pms: " << current_node->PossibleMovesCount() << std::endl;
#endif

  return current_node->NumberOfVisits();
}

//***********************************************************************************************
// Pick best move after all possible moves have been evaluated...
//
// Using highest wins/visits average, but probably could have used highest # of visits as well.
//***********************************************************************************************

void MovesTreeMonteCarlo::PickBestMove(MovesTreeNode *next_move, Board &game_board, Move *suggested_move, bool debug) {
#ifdef DEBUG_BEST_MOVE
  debug = true;
#endif

  if (debug)
    std::cout << "[EngineMonteCarlo::PickBestMove] entered..." << std::endl;

  if (next_move->PossibleMovesCount() == 0) {
    // no possible moves? 'next move' should have outcome properly indicating checkmate or draw...
    assert(next_move->GameOver());
    return;
  }

  MovesTreeNode *high_score_node = NULL;

  float high_score = -100.0;

  for (auto pm = 0; pm < next_move->PossibleMovesCount() && (high_score_node == NULL); pm++) {  
     MovesTreeNode *i = next_move->PossibleMove(pm);
     float this_nodes_win_average = i->NumberOfWins(i->Color()) / i->NumberOfVisits();

     if (debug)
       std::cout << "\tmove:" << Engine::EncodeMove(game_board,*i) 
                 << " # visits: " << i->NumberOfVisits() << ", # wins: " << i->NumberOfWins(i->Color())
                 << ", wins-average: " << this_nodes_win_average
                 << " (" << (roundf(1000 * this_nodes_win_average) / 1000) << ")"
                 << std::endl;
 
     if (i->Outcome() == CHECKMATE) {
       high_score_node = i;
     } else if (this_nodes_win_average > high_score) {
       high_score_node = i;
       high_score = this_nodes_win_average;
     }
  }

  assert(high_score_node != NULL);

  if (debug) {
    std::cout << "[EngineMonteCarlo::PickBestMove] exited. Best move:" << Engine::EncodeMove(game_board,*high_score_node)
              << ", outcome: " << high_score_node->Outcome() << std::endl;
  }

  next_move->Set(high_score_node);
}

//***********************************************************************************************
// UCT - Upper Confident Bound 1 formula
//***********************************************************************************************

class UCB1 {
  public:
  UCB1(float num_wins, int num_visits, float temp, int num_parent_visits) 
    : Wi(num_wins), Si(num_visits), C(temp), Sp(num_parent_visits)
  {
    exploitation_term = Wi / Si;
    exploration_term  = C * sqrt( log(Sp) / Si );

    value = exploitation_term + exploration_term;

    if (isnan(value))
      value = INFINITY;
  }

  float Value() { return value; };

  std::string Parameters() {
    char tbuf[1024];
    sprintf(tbuf,"(Wi=%2.2f Si=%2.2f C=%2.2f Sp=%2.2f exploitation: %2.2f exploration: %2.2f UCB1val: %f)",
            Wi,Si,C,Sp,exploitation_term,exploration_term,value);
    return std::string(tbuf);
  };

  float Wi;
  float Si;
  float C;
  float Sp;
  float exploitation_term;
  float exploration_term;
  float value;
};

//***********************************************************************************************
// Find move with highest score (UCT1 value)...
//
// This method is called once all possible moves for a board state have been explored at least
// once. 
// If ties occur, ie, multiple moves with same score, then moves will be sorted by priority.
//***********************************************************************************************

MovesTreeNode * MovesTreeMonteCarlo::HighScoreMove(float &highest_node_uct, MovesTreeNode *node, MovesTreeNode *parent_node, bool debug) {
#ifdef DEBUG_HIGH_MOVES
  debug = true;
#endif

  if (debug) {
    std::cout << "[HighScoreMove] entered..." << std::endl;
  }

  assert(node->PossibleMovesCount() > 0);

  if (parent_node == NULL)
    parent_node = node;

  highest_node_uct = -1000.0;

  MovesTreeNode *high_score_node = NULL;

  Board game_board;

  int ix = 0;
  for (auto mi = 0; mi != node->PossibleMovesCount(); mi++) {  
    MovesTreeNode *i = node->PossibleMove(mi);

    UCB1 node_ucb(i->NumberOfWins(i->Color()), i->NumberOfVisits(), temperature, parent_node->NumberOfVisits());

    float this_node_uct = node_ucb.Value();
    std::string this_node_uct_parameters = node_ucb.Parameters();

   if (debug) {
     std::cout << "\tUCT1 possible-moves[" << ix++ << "] move: " << Engine::EncodeMove(game_board,*i) 
                << this_node_uct_parameters << std::endl;
    }

    if (this_node_uct > highest_node_uct) {
      high_score_node = i;
      highest_node_uct = this_node_uct;
    }
  }

  assert(high_score_node != NULL); // there is a high-score node, nes pa?
  
  if (debug) {
    std::cout << "[HighScoreMove] exited, high-score: " << highest_node_uct
              << " move: " << Engine::EncodeMove(game_board,*(high_score_node)) << std::endl;
  }

  return high_score_node;
}

}
