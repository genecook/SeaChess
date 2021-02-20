#include <string>
#include <stdexcept>
#include <sstream>
#include <iostream>
#include <algorithm>
#include <cassert>
#include <math.h>

#include <chess.h>
#include <random_moves_game.h>

namespace SeaChess {

//******************************************************************************
// random moves (sub)tree class play the dumbest moves ever!
//******************************************************************************

int MovesTreeRandom::ChooseMove(Move *next_move, Board &game_board, Move *suggested_move) {
  // make up randomized list of all possible moves for the current board/color...
  
  MovesTree moves_engine(Color(),1);

  std::vector<SeaChess::Move> tmoves;

  bool in_check = moves_engine.GetMoves(&tmoves,game_board,Color()); 

  std::random_shuffle( tmoves.begin(), tmoves.end() );

  // select next move...

  bool got_one = false;
  
  for (auto i = tmoves.begin(); i != tmoves.end() && !got_one; i++) {
     MovesTreeNode pm = *i;
     Board updated_board = MovesTree::MakeMove(game_board,&pm);
     if (moves_engine.Check(updated_board,Color())) // don't leave king in check...
       continue;
     next_move->Set(&pm);
     got_one = true;
  }

  // no moves to be made? -- then the current color has been checkmated or played to a draw...

  if (!got_one)
    next_move->SetOutcome(in_check ? RESIGN : DRAW);

  return 1;
}
  
//***********************************************************************************************
// play random game...
//***********************************************************************************************

void RandomMovesGame::Play(float &_white_score, float &_black_score, Board &_current_board, 
                            int _current_color, int _current_level) {
  MovesTreeNode tnode;
  
  current_level  = _current_level;
  starting_level = _current_level;

  PlayInner(&tnode,_current_board,_current_color);

  _white_score = white_score;
  _black_score = black_score;
}

//***********************************************************************************************
// make random moves until game ends...
//***********************************************************************************************

void RandomMovesGame::PlayInner(MovesTreeNode *current_node, Board &current_board, int current_color) {
#ifdef DEBUG_RANDOM_MOVES_GAME
  std::cout << "[RandomMovesGame::PlayInner] entered, color: " << ChessUtils::ColorAsStr(current_color)
            << ", level: " << CurrentLevel() << ".";
#endif

  if (KingsDraw(current_board) || LevelsMaxedOut())
    return;

  // make up randomized list of all possible moves for the current board/color...
  
  MovesTree moves_engine(current_color,CurrentLevel());

  std::vector<SeaChess::Move> tmoves;

  bool in_check = moves_engine.GetMoves(&tmoves,current_board,current_color); 

#ifdef DEBUG_RANDOM_MOVES_GAME
  std::cout << "In check? " << (in_check ? "yes" : "no") << std::endl;
#endif

  std::random_shuffle( tmoves.begin(), tmoves.end() );

  // select next move...
  
  MovesTreeNode pm;     // pm, updated_board will both be valid
  Board updated_board;  //  if a possible move to explore 
  bool got_one = false; //    is identified
  
  for (auto i = tmoves.begin(); i != tmoves.end() && !got_one; i++) {
     pm = *i;                                                // update game board
     updated_board = MovesTree::MakeMove(current_board,&pm); //   with this move
     if (moves_engine.Check(updated_board,current_color))    // don't leave king in check...
       continue;
     got_one = true;
  }

  // no moves to be made? -- then the current color has been checkmated or played to a draw...

  int other_color = (current_color == WHITE) ? BLACK : WHITE;

  if (!got_one) {
    GameEnds(in_check,other_color);
    return;
  }

  // there is a move to explore. add it to the current node, keep playing...

  MovesTreeNode *pvm = current_node->AddMove(pm);

#ifdef DEBUG_RANDOM_MOVES_GAME
  std::string move_chosen = Engine::EncodeMove(current_board,pvm);
  std::cout << "[RandomMovesGame::Play] at level " << CurrentLevel() << " move chosen:" 
            << ChessUtils::ColorAsStr(current_color) << ": " << move_chosen << std::endl;
#endif

  // recursive descent (gasp) 'til game ends... 

  NextLevel();
  PlayInner(pvm,updated_board,other_color);
  PreviousLevel();
}

//***********************************************************************************************
// some utility methods...
//***********************************************************************************************

bool RandomMovesGame::KingsDraw(Board &current_board) {
  if (current_board.TotalPieceCount() == 2) {
#ifdef DEBUG_MONTE_CARLO
    std::cout << "\n[RandomMovesGame::Play] Down to just the two kings, at level " << CurrentLevel() 
              << ". Its a draw" << std::endl;
#endif
    white_score = 0.5;
    black_score = 0.5;
    num_max_levels_reached++;
    return true;
  }
  return false;
}

bool RandomMovesGame::LevelsMaxedOut() {
  if (CurrentLevel() >= MaxLevels()) {
#ifdef DEBUG_MONTE_CARLO
    std::cout << "\n[RandomMovesGame::Play] max levels (" << CurrentLevel() 
              << ") reached - will ASSUME draw" << std::endl;
#endif
    white_score = 0.5;
    black_score = 0.5;
    num_max_levels_reached++;
    return true;
  }
  return false;
}

void RandomMovesGame::GameEnds(bool in_check, int other_color) {
  if (in_check) {
    if (other_color == WHITE) {
      white_score = WIN_SCORE;
      black_score = LOSS_SCORE;
    } else {
      black_score = WIN_SCORE;
      white_score = LOSS_SCORE;
    }
    num_checkmate_outcomes++;
  } else {
    white_score = DRAW_SCORE;
    black_score = DRAW_SCORE;
    num_draw_outcomes++;
  }
  
#ifdef DEBUG_MONTE_CARLO
  if (in_check)
    std::cout << "[RandomMovesGame::Play] game ends in CheckMate!"
              << ", winning color: " << ChessUtils::ColorAsStr(other_color) << ", level: "
      	      << CurrentLevel() << std::endl;
  else    
    std::cout << "[RandomMovesGame::Play] game ends in Draw"
              << ", color: " << ChessUtils::ColorAsStr(other_color) <<std::endl;
#endif
  }
  
}

