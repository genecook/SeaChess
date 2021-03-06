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
extern int master_move_id;
#endif

//#define DEBUG_MONTE_CARLO 1
//#define DEBUG_RANDOM_MOVES_GAME 1
//#define DEBUG_HIGH_MOVES 1
//#define DEBUG_FIXED_RANDOM_SEED 1
//#define DEBUG_BEST_MOVE 1

#define GAMES_BETWEEN_TIMEOUT_CHECKS 1000
  
//***********************************************************************************************
// build up tree of moves; pick the best one. monte-carlo...
//***********************************************************************************************

int MovesTreeMonteCarlo::ChooseMove(Move *next_move, Board &game_board, Move *suggested_move) {
  std::cout << "#  ChooseMove entered, color " << ColorAsStr(Color())
	    << ", turn " << NumberOfTurns() << "..." << std::endl;

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
  SetMaxRandomGameLevels(75);

  // evaluate moves 'til some threshhold reached...

  MovesTreeNode root;

  rollout_index = 0;
  ResetLastLevelVisited();

  StartClock();
  
#ifdef DEBUG_MONTE_CARLO
  std::cout << " max-games-exceeded? " << MaxGamesExceeded() << " timeout? " << Timeout(move_time)
	    << " game over? " << next_move->GameOver() << " rollout-count: " << RolloutCount() << std::endl;
#endif
  
  while( !MaxGamesExceeded() && !Timeout(move_time) && !next_move->GameOver()) {
    for (int i = 0; (i < (GAMES_BETWEEN_TIMEOUT_CHECKS / RolloutCount())) && !MaxGamesExceeded(); i++) {
       float incr_white_wins = 0.0, incr_black_wins = 0.0; 
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

  //GraphMovesToFile("moves", &root); //<---generally only useful when small # of moves possible

  // at this point, the list of possible moves (moves explored) attached to the 'next' move
  // represents the list of potential moves...

  PickBestMove(&root,game_board,suggested_move);

  next_move->Set(&root);

#ifdef DEBUG_MONTE_CARLO
  float highest_node_uct;
  HighScoreMove(highest_node_uct,&root,&root,true);
#endif

  return TotalGamesCount();
}

void MovesTreeMonteCarlo::ChooseMoveInner(MovesTreeNode *node, float &incr_white_wins, float &incr_black_wins,
					  Board &current_board,int current_color) {
#ifdef DEBUG_MONTE_CARLO
    std::cout << "[EngineMonteCarlo::ChooseMoveInner] entered, color: " 
              << ColorAsStr(current_color) << ", level: " 
              << Levels() << " # visits:" << node->NumberOfVisits()
              << ", previous move: (" << Engine::EncodeMove(current_board,*node) << ")"
              << " # possible-moves: " << node->PossibleMovesCount() << "..." << std::endl;
#endif

  node->IncrementVisitCount();

  UpdateLastLevel(Levels());

  // is it a draw?

  if (MaxLevelsReached()) {
#ifdef DEBUG_MONTE_CARLO
    std::cout << "[EngineMonteCarlo::ChooseMoveInner] max-levels reached. Game ends in a draw." << std::endl;
#endif
    BumpTotalGamesCount(); // this counts of course as a completed game
    node->SetOutcome(DRAW);
    return;
  }

  // add list of possible moves for this game state, if needed...

  if (node->PossibleMovesCount() == 0) {
    // populate possible moves list...
#ifdef DEBUG_MONTE_CARLO
    std::cout << "[EngineMonteCarlo::ChooseMoveInner] get list of all possible moves for this game state..."
	      << std::endl;
#endif
    // add valid moves to node...

    bool in_check = GetMoves(node,current_board,current_color,true /* avoid check */,true /* sort moves */);
#ifdef DEBUG_MONTE_CARLO
    std::cout << "[EngineMonteCarlo::ChooseMoveInner] there are " << node->PossibleMovesCount() 
              << " possible moves for this game state..." << std::endl;
#endif
     // no valid moves? then assume draw or checkmate
    if (node->PossibleMovesCount() == 0) {
      int win_color = (current_color == WHITE) ? BLACK : WHITE;
      if (in_check)
        node->SetOutcome(win_color == Color() ? CHECKMATE : RESIGN);
      else
        node->SetOutcome(DRAW);
      BumpTotalGamesCount(); // this counts of course as a completed game
#ifdef DEBUG_MONTE_CARLO
      std::cout << "[EngineMonteCarlo::ChooseMoveInner] game ends in " << (in_check ? "CheckMate!" : "Draw!")
		<< ", winning color: " << ColorAsStr(win_color) <<std::endl;
#endif
      return;
    }
  }

  // explore based on the most promising move (select move with highest UCT)...

  float highest_node_uct = 0.0;

  MovesTreeNode *next_move = HighScoreMove(highest_node_uct,node);

  assert(next_move != NULL); // there is a high score, nes pa?

#ifdef DEBUG_MONTE_CARLO
  std::cout << "[EngineMonteCarlo::ChooseMoveInner] next move: " << (*next_move) << std::endl;
#endif
  
  Board updated_board = MovesTree::MakeMove(current_board, next_move);

  // we haven't visited this node before, do rollout and return...
  
  if (next_move->NumberOfVisits() == 0) {
    Rollout(next_move, updated_board, current_board, OtherColor(current_color));
    incr_white_wins = next_move->NumberOfWhiteWins();
    incr_black_wins = next_move->NumberOfBlackWins();
    next_move->IncrementVisitCount();
#ifdef DEBUG_MONTE_CARLO
    std::cout << "[EngineMonteCarlo::ChooseMoveInner] returns from leaf node 'addition', incr wins white/black: " 
              << incr_white_wins << "/" << incr_black_wins << "..." << std::endl;
#endif
    return;
  }

  // descend game tree to next level...

  NextLevel();
  
#ifdef DEBUG_MONTE_CARLO
  std::cout << "  ChooseMoveInner descending, next level: " << Levels() << "..." << std::endl;
#endif
  
  ChooseMoveInner(next_move, incr_white_wins, incr_black_wins, updated_board, OtherColor(current_color));

  node->IncreaseWinsCounts( incr_white_wins, incr_black_wins );

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
  std::cout << "[EngineMonteCarlo::Rollout] entered for color " << ColorAsStr(current_color) << "..." << std::endl;
#endif

  assert(current_node->PossibleMovesCount() == 0); // this node hasn't been visited yet, nes pa?

  // play N games from this node (move) to yield an aggregate score...

  bool in_check = false;

  for (auto i = 0; i < RolloutCount(); i++) {
     SeaChess::RandomMovesGame rndgame(MaxRandomGameLevels(),NumberOfTurns());
     float white_score = 0.0, black_score = 0.0;
     rndgame.Play(white_score,black_score,current_board,current_color,Levels());
#ifdef DEBUG_MONTE_CARLO
     std::cout << "[EngineMonteCarlo::rollout] random game wh/bl wins: " << white_score << "/" << black_score << std::endl;
#endif
     current_node->IncreaseWinsCounts( white_score, black_score );
     BumpTotalGamesCount();
     int num_draws, num_checkmates, num_max_levels;
     rndgame.RandomGameStats(num_draws, num_checkmates, num_max_levels); 
     UpdateRandomGameStats(num_draws, num_checkmates, num_max_levels); 
#ifdef DEBUG_MONTE_CARLO
     std::cout << "[EngineMonteCarlo::rollout] current node wh/bl wins: " << current_node->NumberOfWhiteWins()
               << "/" << current_node->NumberOfBlackWins() << std::endl;
#endif
  }

#ifdef DEBUG_MONTE_CARLO
  std::cout << "[EngineMonteCarlo::rollout] exited, moves count: " << current_node->NumberOfVisits() << std::endl;
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
    std::cout << "[EngineMonteCarlo::PickBestMove] entered, suggested move: "
	      << *suggested_move << ", color: " << ColorAsStr(suggested_move->Color()) << "..." << std::endl;

  if (next_move->PossibleMovesCount() == 0) {
    // no possible moves? 'next move' should have outcome properly indicating checkmate or draw...
    assert(next_move->GameOver());
    return;
  }

  MovesTreeNode *high_score_node = NULL;

  float high_score = -100000.0;

  bool have_suggested_move = (suggested_move != NULL);
  bool allow_suggested_move = have_suggested_move;
  
  for (auto pm = 0; pm < next_move->PossibleMovesCount(); pm++) {  
     MovesTreeNode *i = next_move->PossibleMove(pm);
     float this_nodes_win_average = i->NumberOfWins(i->Color()) / i->NumberOfVisits();

     if (debug)
       std::cout << "\tmove:" << Engine::EncodeMove(game_board,*i)
	         << ", color: " << ColorAsStr(i->Color())
                 << ", # visits: " << i->NumberOfVisits() << ", # wins: " << i->NumberOfWins(i->Color())
                 << ", wins-average: " << this_nodes_win_average
                 << " (" << (roundf(1000 * this_nodes_win_average) / 1000) << ")"
	         << " outcome: " << OutcomeAsStr(i->Outcome())
                 << std::endl;
 
     if (i->Outcome() == CHECKMATE) {
       high_score_node = i;
       break;
     }

     if (this_nodes_win_average > high_score) {
       high_score_node = i;
       high_score = this_nodes_win_average;
     }

     // any move that is not a simple-move makes the use of the suggested move (more) uncertain...
     
     allow_suggested_move &= (i->Outcome() == SIMPLE_MOVE); 

     have_suggested_move |= (*i == *suggested_move); // suggested move is in the mix
  }

  assert(high_score_node != NULL); // there must have been a high score node, es verdad?

  bool use_suggested_move = allow_suggested_move && have_suggested_move;
  
  Move *best_move = use_suggested_move ? suggested_move : high_score_node;
  
  if (debug) {
    std::cout << "[EngineMonteCarlo::PickBestMove] exited. Best move:"
	      << Engine::EncodeMove(game_board,*best_move) << std::endl;
  }

  next_move->Set(best_move);
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

MovesTreeNode * MovesTreeMonteCarlo::HighScoreMove(float &highest_node_uct, MovesTreeNode *node,
						   MovesTreeNode *parent_node, bool debug) {
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
