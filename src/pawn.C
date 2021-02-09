#include <string>
#include <stdexcept>
#include <iostream>
#include <assert.h>

#include <chess.h>

namespace SeaChess {

//***********************************************************************************************
// Moves - for this piece, append to existing list all valid moves given the current board state,
//         that are either not blocked or occupied by a piece that can be taken...
//***********************************************************************************************

void Pawn::Moves( std::vector<Move> *moves, Board &the_board, int color, int row, int column ) {
  int row_increment = (color == WHITE) ? 1 : -1; // white pawns move up; black pawns move down...

  // check for en passant from the current position...

  if ( the_board.EnPassantSet(row,column - 1,color) ) {
    EvalMoveTo(moves,the_board,color,row,column,row + row_increment,column - 1,false,false,true);
  }
  
  if ( the_board.EnPassantSet(row,column + 1,color) ) {
    EvalMoveTo(moves,the_board,color,row,column,row + row_increment,column + 1,false,false,true);
  }

  // pawn can move up one row at a time. if this is the pawns starting row, then it can
  // move two rows up as well...

  bool on_starting_row = the_board.StartingRow(row,color);
  
  bool can_capture = false;  // these 1st two moves are assumed to not capture pieces...

  int end_row = row + row_increment;
  int move_outcome = EvalMoveTo(moves,the_board,color,row,column,end_row,column,can_capture);
  bool move_good = (move_outcome == SIMPLE_MOVE);

  // see if 1st move results in check...
  if (move_good) {
    if (the_board.EndingRow(end_row,color)) {
      moves->back().SetOutcome(PROMOTION);
      if (CheckOnPromotion(the_board,end_row,column,color)) {
        moves->back().SetCheck();
      }
    } else {
      // possible pawn check on king...
      if ( (the_board.OpposingKing(end_row + row_increment,column - 1,color)
		     || the_board.OpposingKing(end_row + row_increment,column + 1,color)) ) {
        moves->back().SetCheck();
      }
      // this position could block opponents ability to castle...
      else if (!on_starting_row)
        EvalBlocksCastling(moves,the_board,color,end_row,column); 
    }
  }

  if ( on_starting_row && move_good ) {
    // if pawn is on starting row (by color) and the 1st possible move is possible,
    //   then the 2nd move is also possible...
    end_row += row_increment;
    int move_outcome = EvalMoveTo(moves,the_board,color,row,column,end_row,column,can_capture);
    move_good = (move_outcome == SIMPLE_MOVE);
    // see if 2nd move results in check...  
    if ( move_good && (the_board.OpposingKing(end_row + row_increment,column - 1,color)
		       || the_board.OpposingKing(end_row + row_increment,column + 1,color)) ) {
      moves->back().SetCheck();
    }
  }

  // pawn can, from its current position, also capture on its diagonals... 

  can_capture = true;        // can capture of course
  bool must_capture = true;  // only valid if a capture occurs

  // check 1st diagonal...
  
  int capture_row = row + row_increment;

  int capture_column = column - 1;
  move_outcome = EvalMoveTo(moves,the_board,color,row,column,capture_row,capture_column,
			    can_capture, must_capture); // next row, column - 1;
  move_good = (move_outcome == CAPTURE);

  // from 1st capture position look for check:
  if ( move_good) {
    if (the_board.EndingRow(capture_row,color)) {
      moves->back().SetOutcome(PROMOTION);
      if (CheckOnPromotion(the_board,capture_row,capture_column,color)) {
        moves->back().SetCheck();
      }
    } else if ( the_board.OpposingKing(capture_row + row_increment,capture_column - 1,color)
	        || the_board.OpposingKing(capture_row + row_increment,capture_column + 1,color) ) {
      moves->back().SetCheck();
    } else {
      // this position could block opponents ability to castle...
      EvalBlocksCastling(moves,the_board,color,capture_row,capture_column); 
    }
  }

  // check 2nd diagonal...
  
  capture_column = column + 1; // capture row same
  
  move_outcome = EvalMoveTo(moves,the_board,color,row,column,capture_row,capture_column,
			    can_capture, must_capture);
  move_good = (move_outcome == CAPTURE);

  // from 2nd capture position look for check:
  capture_column = column - 1; // capture row same

  if (move_good) {
    if (the_board.StartingRow(capture_row,color)) {
      moves->back().SetOutcome(PROMOTION);
      if (CheckOnPromotion(the_board,capture_row,capture_column,color)) {
        moves->back().SetCheck();
      }
    } else if ( the_board.OpposingKing(capture_row + row_increment,capture_column - 1,color)
	       || the_board.OpposingKing(capture_row + row_increment,capture_column + 1,color) ) {
      moves->back().SetCheck();
    } else {
      // this position could block opponents ability to castle...
      EvalBlocksCastling(moves,the_board,color,capture_row,capture_column);
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

bool Pawn::Check(Board &the_board, int kings_row, int kings_column, int color, int row, int column) {
  int row_increment = (color == WHITE) ? 1 : -1;   // white pawns move up; black pawns move down...

  // check 'next' diagonal from pawns position...

  bool in_check = false;
  
  if ( ((row + row_increment) == kings_row) && ((column - 1) == kings_column) )
    in_check = true;
  else if ( ((row + row_increment) == kings_row) && ((column + 1) == kings_column) )
    in_check = true;

  return in_check;
}

//***********************************************************************************************
// from the row/column position, does a pawn prevent the opponent from castling?
//***********************************************************************************************

void Pawn::EvalBlocksCastling(std::vector<Move> *moves, Board &the_board, int color, int row, int column) {
  assert(moves->size() > 0);

  int row_increment = (color == WHITE) ? 1 : -1;   // white pawns move up; black pawns move down...

  moves->back().SetCastlingBlocked(the_board.EvalBlockCastling(color,row + row_increment,column + 1));
  moves->back().SetCastlingBlocked(the_board.EvalBlockCastling(color,row + row_increment,column - 1));
}

//***********************************************************************************************
//***********************************************************************************************

bool Pawn::CheckOnPromotion(Board &the_board, int end_row, int column, int color) {
  // pawn promotion. will ASSUME to queen). see if new queen checks opposing king...
  
  int kings_row = 0,kings_column = 0,kings_color = (color == WHITE) ? BLACK : WHITE;
      
  the_board.GetKing(kings_row,kings_column,kings_color);
  
  return ( ChecksDiagonal(the_board,kings_row,kings_column,color,end_row,column)
	   || ChecksHorizVert(the_board,kings_row,kings_column,color,end_row,column) );
}

}

