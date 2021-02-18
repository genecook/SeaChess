#ifndef __RANDOM_GAME__

#include <chess.h>

//***********************************************************************************************
// play a single random game of chess to conclusion or until max-levels reached
// (in which case a draw).
//***********************************************************************************************

#define WIN_SCORE  1.0
#define LOSS_SCORE 0.0
#define DRAW_SCORE 0.5

#define TURNS_THRESHHOLD 10

namespace SeaChess {
  
class RandomMovesGame {
  public:
    RandomMovesGame(unsigned int _max_levels, unsigned int _turn_number = TURNS_THRESHHOLD) 
          : max_levels(_max_levels), turn_number(_turn_number),
            white_score(0.0), black_score(0.0),num_draw_outcomes(0), num_checkmate_outcomes(0), num_max_levels_reached(0) { 
    };

    ~RandomMovesGame() {
    };

    void Play(float &_white_score, float &_black_score, Board &_current_board, int _current_color, int _current_level);

    void PlayInner(MovesTreeNode *_current_node, Board &_current_board, int _current_color);

    void PredictOutcome(Board &current_board, int current_color);

    int StartingLevel() { return starting_level; };
    int CurrentLevel() { return current_level; };
    void NextLevel() { current_level++; };
    void PreviousLevel() { current_level--; };
    int MaxLevels() { return max_levels; };

    void RandomGameStats(int &_num_draws, int &_num_checkmates, int &_num_max_levels_reached) {
      _num_draws = num_draw_outcomes;
      _num_checkmates = num_checkmate_outcomes;
      _num_max_levels_reached = num_max_levels_reached;
    };

    bool KingsDraw(Board &current_board);
    bool LevelsMaxedOut();
    void GameEnds(bool in_check, int other_color);

  private:
    unsigned int current_level;     // current level
    unsigned int starting_level;    // starting level

    unsigned int max_levels;        // maximum # of levels to play before draw
    unsigned int turn_number;       // if turn# < play threshhold, return statistical outcome instead of actual play

    float        white_score;       // scores
    float        black_score;       //   after game concludes

    int num_draw_outcomes;          // # of random games that ended in draw
    int num_checkmate_outcomes;     //       "                "        checkmate
    int num_max_levels_reached;     //       "                "     when max-levels reached
};

};

#endif
#define __RANDOM_GAME__ 1


