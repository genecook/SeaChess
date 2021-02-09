#include <string>
#include <stdexcept>
#include <unistd.h>
#include <iostream>

#include <chess.h>

namespace SeaChess {

// foreach piece - return list of all valid moves given the current board state,
//                 that are either not blocked or occupied by a piece that can
//                 be taken...

void King::Moves( std::vector<Move> *moves, Board &the_board, int color, int row, int column ) {
  
  the_board.GetOpposingKing(opponents_king_row,opponents_king_column,color);
  
  for (auto i = 0; i < King::NUM_MOVES; i++) { // foreach possible king move...
     int end_row = row + rows[i];
     int end_col = column + cols[i];
     if (MoveCausesCheck(the_board,end_row,end_col,color))
       continue;
     EvalMoveTo(moves,the_board,color,row,column,end_row,end_col);
  }

  // when gathering moves, the kings moves must be gathered last (gag me),
  // to allow any other move to set 'castling blocked' state...
  
  bool castling_blocked = false;
  for (auto pmi = moves->begin(); pmi != moves->end(); pmi++) {
    castling_blocked |= (*pmi).CastlingBlocked(color);
  }
  
  if (castling_blocked) {
    // can't castle - opponents piece 'covering' a castle square...
  } else {
    int end_col = 0;

    if ((end_col = the_board.CastleValid(color,true /* = kings side castling */)) != 0)
      EvalMoveTo(moves,the_board,color,row,column,row,end_col);
    if ((end_col = the_board.CastleValid(color,false /* = queens side castling */)) != 0)
      EvalMoveTo(moves,the_board,color,row,column,row,end_col);
  }
}

//***********************************************************************************************
// Check - for this piece, is the opposing king in check.
//
//  kings_row,kings_column,color - opposing kings position, color
//  row,column - pawns position
//
// NOTE: locations for king and pawn are ASSUMED to be valid!
// NOTE2: King should not be able to actually check the opposing king!
//***********************************************************************************************

bool King::Check(Board &the_board, int kings_row, int kings_column, int color, int row, int column) {
  //throw std::logic_error("The king may not directly check the opposing players king!");

  for (auto i = 0; i < King::NUM_MOVES; i++) { // foreach possible king move...
    if ( ((kings_row + rows[i]) == row) && ((kings_column + cols[i]) == column) )
      return true;
  }
  
  return false;
}

bool King::MoveCausesCheck(Board &the_board, int kings_row, int kings_column,int color) {
  return Check(the_board,kings_row,kings_column,color,opponents_king_row,opponents_king_column);
}

}

