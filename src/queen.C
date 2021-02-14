#include <string>
#include <stdexcept>
#include <unistd.h>
#include <iostream>

#include <chess.h>

namespace SeaChess {

// foreach piece - return list of all valid moves given the current board state,
//                 that are either not blocked or occupied by a piece that can
//                 be taken...

void Queen::Moves( std::vector<Move> *moves, Board &the_board, int color, int row, int column, bool in_check ) {
  // the queen is a bad mamma-jamma...

  MovesDiagonal(moves,the_board,color,row,column);
  MovesHorizVert(moves,the_board,color,row,column);
}

// Check - for this piece, is the opposing king in check.
//
//  kings_row,kings_column,color - opposing kings position, color
//  row,column - pawns position
//
// NOTE: locations for king and pawn are ASSUMED to be valid!

bool Queen::Check(Board &the_board, int kings_row, int kings_column, int color, int row, int column) {
  if (ChecksDiagonal(the_board,kings_row,kings_column,color,row,column)) {
    return true;
  }
  
  if (ChecksHorizVert(the_board,kings_row,kings_column,color,row,column)) {
    return true;
  }

  return false;
}

}

