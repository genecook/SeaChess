#include <string>
#include <stdexcept>
#include <unistd.h>
#include <iostream>

#include <chess.h>

namespace SeaChess {

// foreach piece - return list of all valid moves given the current board state,
//                 that are either not blocked or occupied by a piece that can
//                 be taken...

void Knight::Moves( std::vector<Move> *moves, Board &the_board, int color, int row, int column, bool in_check ) {
  int rows[] = { 2,  2,  1, -1, -2, -2,  1, -1 };
  int cols[] = { 1, -1,  2,  2,  1, -1, -2, -2 };

  for (auto i = 0; i < 8; i++) { // foreach possible knight move...
     int end_row = row + rows[i];
     int end_column = column + cols[i];
     int move_outcome = EvalMoveTo(moves,the_board,color,row,column,end_row,end_column);
     
     if ( (move_outcome == SIMPLE_MOVE) || (move_outcome == CAPTURE) ) {
       // if the move is possible, then from this position, ie, from the next 8 possible moves, see if the opposing 
       // king will be in check...
       bool is_in_check = false;
       for (auto j = 0; (j < 8) && !is_in_check; j++) {
	  if (the_board.OpposingKing(end_row + rows[j],end_column + cols[j],color)) {
	    moves->back().SetCheck();
	    is_in_check = true;
          }
       }
     }
  }
}

//***********************************************************************************************
// Check - for this piece, is the opposing king in check.
//
//  kings_row,kings_column,color - opposing kings position, color
//  row,column - pawns position
//
// NOTE: locations for king and pawn are ASSUMED to be valid!
//***********************************************************************************************

bool Knight::Check(Board &the_board, int kings_row, int kings_column, int color, int row, int column) {
  int rows[] = { 2,  2, -2, -2, 1, -1,  1, -1 };
  int cols[] = { 1, -1,  1, -1, 2,  2, -2, -2 };

  bool check = false;
  
  for (auto i = 0; (i < 8) && !check; i++) { // foreach possible knight move...
    int check_row = row + rows[i];
    int check_column = column + cols[i];
    check = (check_row == kings_row) && (check_column == kings_column);
  }
  
  return check;
}

}

