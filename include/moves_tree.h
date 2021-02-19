#ifndef __MOVETREE__

//******************************************************************************
// MovesTree
//******************************************************************************

#include <string>
#include <iostream>
#include <stdlib.h>
#include <vector>
#include <algorithm>
#include <sys/time.h>
#include <math.h>

//#define GRAPH_SUPPORT 1

namespace SeaChess {

#ifdef GRAPH_SUPPORT
extern int master_move_id;
#endif
  
//******************************************************************************
// moves tree node...
//******************************************************************************

class MovesTreeNode : public Move {
public:
  MovesTreeNode() : possible_moves(NULL) {
    InitMove();
#ifdef GRAPH_SUPPORT
    move_id = master_move_id++;
#endif
  };
  
  MovesTreeNode(int start_row, int start_column, int end_row, int end_column, int color,
		int outcome = INVALID_INDEX, int capture_type = INVALID_INDEX) : possible_moves(NULL) {
    InitMove(start_row, start_column, end_row, end_column, color, outcome, capture_type);
#ifdef GRAPH_SUPPORT
    move_id = master_move_id++;
#endif
  };

  MovesTreeNode(Move move) : possible_moves(NULL) {
    InitMove(move.StartRow(), move.StartColumn(), move.EndRow(), move.EndColumn(),
	     move.Color(), move.Outcome(), move.Check(), move.CaptureType());
#ifdef GRAPH_SUPPORT
    move_id = master_move_id++;
#endif
  };
  
  ~MovesTreeNode() { Flush(); };

  MovesTreeNode *AddMove(Move new_move) {
    assert ( (pm_count + 1) < INT_LEAST8_MAX );
    
    MovesTreeNode *new_node = (MovesTreeNode *) malloc( sizeof(MovesTreeNode) );
    new_node->Set(&new_move);
    new_node->pm_count = 0;
    new_node->possible_moves = NULL;
    possible_moves = (MovesTreeNode **) realloc(possible_moves, sizeof(MovesTreeNode *) * (pm_count + 1) );
    possible_moves[pm_count] = new_node;
    pm_count++;
    return new_node;
  };
  
  int PossibleMovesCount() { return pm_count; };
  
  MovesTreeNode *PossibleMove(int index) { return possible_moves[index]; };

  void Flush() {
    if (pm_count > 0) {
      assert(possible_moves != NULL);
      for (int i = 0; i < pm_count; i++) {
         possible_moves[i]->Flush();
         free(possible_moves[i]);
      }
      free(possible_moves);
      pm_count = 0;
      possible_moves = NULL;
    } else {
      assert( (pm_count == 0) && (possible_moves == NULL) );
    }
  };

  void Sort( bool (*sortfunction)(MovesTreeNode *m1, MovesTreeNode *m2) ) {
    std::sort( possible_moves, possible_moves + pm_count, sortfunction );
  };

  void Randomize() {
    std::random_shuffle( possible_moves, possible_moves + pm_count );
  };

int NumberOfVisits() { return num_node_visits; };

  float NumberOfWins(int color) { return (color == WHITE) ? num_white_wins : num_black_wins; };

  float NumberOfWhiteWins() { return num_white_wins; };
  float NumberOfBlackWins() { return num_black_wins; };

  void DisplayWins(const std::string &prefix) {
    std::cout << prefix << " #visits: " << NumberOfVisits() << ", # white wins:" << NumberOfWhiteWins()
              << ", # black wins:" << NumberOfBlackWins() << std::endl;
  };

  void IncrementVisitCount(int _increment = 1) { num_node_visits += _increment; };
  void SetVisitCount(int _increment = 1) { num_node_visits = _increment; };

  void ScoreWinsCount(float &incr_white_wins, float &incr_black_wins, int _color, bool in_check = true) {
    if (in_check) {
      // a win...
      if (_color == WHITE) {
        num_white_wins += 1.0;
        incr_white_wins = 1.0;
      } else {
        num_black_wins += 1.0;
        incr_black_wins = 1.0;
      }
    } else {
        // a draw...
        num_black_wins += 0.5;
        num_white_wins += 0.5;
        incr_white_wins = 0.5;
        incr_black_wins = 0.5;
    }
  };

  void SetWinCounts(float &incr_white_wins, float &incr_black_wins, float _white_wins, float _black_wins) {
      num_white_wins += _white_wins;
      num_black_wins += _black_wins;
      incr_white_wins = _white_wins;
      incr_black_wins = _black_wins;
  };

  void IncreaseWinsCounts( float _white_wins, float _black_wins) {
      num_white_wins += _white_wins;
      num_black_wins += _black_wins;
  };

#ifdef GRAPH_SUPPORT
  int ID() { return move_id; };
  int move_id;
#endif
  
private:
  //int_least8_t pm_count;

  int   num_node_visits;             // 
  float num_white_wins;              // used in monte-carlo tree simulation
  float num_black_wins;              //

  MovesTreeNode **possible_moves;
};


struct piece_counts {
    piece_counts() : kings(0), queens(0), bishops(0),knights(0),rooks(0),pawns(0) {};

    void dump(const std::string &prefix) {
      std::cout << prefix << "# kings/queens/bishops/knights/rooks/pawns: " << kings
      << "/" << queens << "/" << bishops << "/" << knights << "/" << rooks
      << "/" << pawns << std::endl;
    };

    int kings;
    int queens;
    int bishops;
    int knights;
    int rooks;
    int pawns;
};

//******************************************************************************
// base moves tree class...
//******************************************************************************

class MovesTree {
 public:
  MovesTree(int _color, int _max_levels) : color(_color), max_levels(_max_levels) {
    root_node = new MovesTreeNode;
  };
  
  ~MovesTree() { root_node->Flush(); };

  virtual int ChooseMove(Move *next_move, Board &game_board, Move *suggested_move = NULL) { return 0; };
  bool GetMoves(std::vector<Move> *possible_moves, Board &game_board, int color,bool avoid_check = true);
  bool Check(Board &board,int color);
  
  static Board MakeMove(Board &board, MovesTreeNode *pv);

 protected:
  void EvalBoard(MovesTreeNode *move, Board &current_board, int forced_score=UNKNOWN);
  int MaterialScore(Board &current_board);
  
  bool GetMoves(MovesTreeNode *current_node, Board &current_board, int current_color,
		bool avoid_check = true, bool sort_moves = false);

  void PickBestMove(MovesTreeNode *root_node, Board &game_board, Move *suggested_move);
  bool BestScore(MovesTreeNode *this_move, MovesTreeNode *previous_move);

  void CountPieces(struct piece_counts &counts, Board &game_board,int color);
  int GetPieceCount(Move *node,Board &game_board,int color);

  int Color() { return color; };
  int OtherColor(int current_color) { return (current_color == WHITE) ? BLACK : WHITE; };
  int NextColor(int current_color) { return OtherColor(current_color);  };
  
  int MaxLevels() { return max_levels; };

  void GraphMovesToFile(const std::string &outfile, MovesTreeNode *node);
  void GraphMoves(std::ofstream &grfile, MovesTreeNode *node, int level);

  MovesTreeNode *root_node;

  int color;
  int max_levels;

  int eval_count;

  int kings_row;
  int kings_column;

  Pieces pieces; // used to generate moves for each chess piece type
};

//******************************************************************************
// minimax moves (sub)tree class...
//******************************************************************************

class MovesTreeMinimax : public MovesTree {
 public:
  MovesTreeMinimax(int _color, int _max_levels) : MovesTree(_color,_max_levels) {};

  int ChooseMove(Move *next_move, Board &game_board, Move *suggested_move = NULL);

 private:

  void ChooseMoveInner(MovesTreeNode *current_node, Board &current_board, int current_color,
		       int current_level, int alpha, int beta);
};

//******************************************************************************
// monte-carlo moves (sub)tree class...
//******************************************************************************

class MovesTreeMonteCarlo : public MovesTree {
 public:
  MovesTreeMonteCarlo(int _color, int _max_levels, int _move_time)
    : MovesTree(_color,_max_levels), move_time(_move_time), num_turns(0), total_games_count(0),
      max_games_count(-1),number_of_levels(0),max_levels(0), num_draw_outcomes(0),
      num_checkmate_outcomes(0), num_max_levels_reached(0), max_random_game_levels(0),
      move_root(NULL), last_level(0), temperature(1.5), rollout_index(0), rollout_count(1) {
  };

  int  ChooseMove(Move *next_move, Board &game_board, Move *suggested_move = NULL);

  void PickBestMove(MovesTreeNode *next_move, Board &game_board, Move *suggested_move, bool debug = false);
  MovesTreeNode * HighScoreMove(float &highest_node_uct, MovesTreeNode *node, MovesTreeNode *parent_node = NULL, bool debug = false);
  int Rollout(MovesTreeNode *current_node, Board &current_board, Board &previous_board, int current_color);
    
  int  NumberOfTurns()        { return num_turns; };
  int  BumpNumberOfTurns()    { num_turns++; return num_turns; };
  int  TotalGamesCount()      { return total_games_count; };
  void ResetTotalGamesCount() { total_games_count = 0; };
  int  BumpTotalGamesCount()  { total_games_count++; return total_games_count; };
  
  void SetLevels(int _levels) { number_of_levels = _levels; };
  int  Levels()               { return number_of_levels; };
  int  NextLevel()            { number_of_levels++; return number_of_levels; };
  int  PreviousLevel()        { number_of_levels--; return number_of_levels; };
  int  MaxLevels()            { return max_levels; };
  void SetMaxLevels(int nval) { max_levels = nval; };
  bool MaxLevelsReached()     { return Levels() == MaxLevels(); };

  int  MaxRandomGameLevels() { return max_random_game_levels; };
  void SetMaxRandomGameLevels(int nval) { max_random_game_levels = nval; };
  bool MaxRandomGameLevelsReached() { return max_random_game_levels; };

  int  LastLevelVisited() { return last_level; };
  void ResetLastLevelVisited() { last_level = 0; };
  void UpdateLastLevel(int _level) { if (_level > last_level) last_level = _level; };

  int MaxGamesCount() { return max_games_count; };
  bool MaxGamesExceeded() {
    return MaxGamesCount() > 0 ? (TotalGamesCount() > MaxGamesCount()) : false;
  };

  int RolloutCount() { return rollout_count; };
  
  void ResetRandomGameStats() {
    num_draw_outcomes = 0;
    num_checkmate_outcomes = 0;
    num_max_levels_reached = 0;
  };

  void UpdateRandomGameStats(int &_num_draws, int &_num_checkmates, int &_num_max_levels_reached) {
    num_draw_outcomes += _num_draws;
    num_checkmate_outcomes += _num_checkmates;
    num_max_levels_reached += _num_max_levels_reached;
  };

 void RandomGameStats(int &_num_draws, int &_num_checkmates, int &_num_max_levels_reached) {
    _num_draws = num_draw_outcomes;
    _num_checkmates = num_checkmate_outcomes;
    _num_max_levels_reached = num_max_levels_reached;
  };

 void StartClock() {
    gettimeofday(&t1,NULL);
  };

  double ElapsedTime() {
    struct timeval t2;
    gettimeofday(&t2,NULL);
    return ((t2.tv_sec - t1.tv_sec) * 1000.0); // milliseconds part is close enough
  };

  bool Timeout(unsigned int move_time_in_seconds) {
    return ElapsedTime() > (double) move_time_in_seconds * 1000.0;
  };

 private:

  void ChooseMoveInner(MovesTreeNode *current_node, float &incr_white_wins, float &incr_black_wins, Board &current_board, int current_color);

  int move_time;              // in seconds
  unsigned int num_turns;     // # of turns in a game (i move, then you move...)
  int total_games_count;      // total # of games played
  int max_games_count;        // max # of games to play
  int number_of_levels;       // current level
  int max_levels;             // max levels to traverse
  int num_draw_outcomes;      //
  int num_checkmate_outcomes; // random game stats
  int num_max_levels_reached; //
  int max_random_game_levels; // max levels to traverse in random games
  int last_level;             // deepest level explored

  float temperature;
  
  MovesTreeNode *move_root;
  int rollout_index;
  int rollout_count;
  
  struct timeval t1;          // used to time moves
  double elapsed_time;

};

};

#endif
#define __MOVETREE__
