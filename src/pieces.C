#include <string>
#include <stdexcept>
#include <iostream>
#include <assert.h>

#include "chess.h"

namespace SeaChess {
  
//***********************************************************************************************
//***********************************************************************************************

void Piece::AddPossibleMove(std::vector<Move> *moves, int start_row, int start_column, int end_row, int end_column, int color,
                            int outcome, int capture_type) {
  assert( (outcome != CAPTURE) || (capture_type != UNKNOWN) );
  
  moves->push_back( Move(start_row,start_column,end_row,end_column,color,outcome,capture_type) );

  assert( (moves->back().Outcome() != CAPTURE) || (moves->back().CaptureType() != UNKNOWN) );
  
}


//***********************************************************************************************
// evaluate move - is the proposed move location (board square index) valid? is the square
// occupied?...
//***********************************************************************************************

int Piece::EvalMoveTo(std::vector<Move> *moves, Board &the_board, int color, int start_row, int start_column, int moveto_row,
	int moveto_column, bool can_capture, bool must_capture, bool en_passant) {
#ifdef DEBUG_ENGINE
  std::cout << "[EvalMoveTo] entered, color: " << ChessUtils::ColorAsStr(color)
	    << " start position: " << the_board.Coordinates(start_row,start_column)
	    << " proposed move-to position: " << the_board.Coordinates(moveto_row,moveto_column)
	    << std::endl;
#endif
  
  if (!the_board.ValidRow(moveto_row) || !the_board.ValidColumn(moveto_column)) {
#ifdef DEBUG_ENGINE

    std::cout << "[EvalMoveTo] moveto row/column is off the board." << std::endl;
#endif
    return INVALID_INDEX; // oops! we're off the board...
  }
  
  // board/square index is valid...

  int other_piece_type,other_piece_color;
  
  bool some_other_piece = the_board.GetPiece(other_piece_type,other_piece_color,moveto_row,moveto_column);

  if (some_other_piece && (other_piece_color == color) ) {
    // square occupied by piece of same color...
#ifdef DEBUG_ENGINE
    std::cout << "[EvalMoveTo] move is blocked by another piece" << std::endl;
#endif
    return SQUARE_BLOCKED;
  }

  if (en_passant) {
    AddPossibleMove(moves,start_row,start_column,moveto_row,moveto_column,color,CAPTURE,PAWN);
#ifdef DEBUG_ENGINE
    std::cout << "[EvalMoveTo] move results in 'en-passant' capture" << std::endl;
#endif
    
    return CAPTURE;
  }
  
  if (some_other_piece && (other_piece_color != color) ) {
    // square occupied by opponents piece...
    if (can_capture) {
      // piece can be captured...
      if (other_piece_type == KING) {
	      if (SpeculativeMode()) {
	        // gathering moves for speculation only...
	      } else {
          // if king is taken, then we missed king-in-check state on previous move...
	        //std::cout << "# game board:\n" << the_board << std::endl;
          throw std::logic_error("king is taken???");
	        //std::cout << "TAKE KING???" << std::endl;
	        return INVALID_INDEX;
	      }
      }
      if (other_piece_type == NONE)
	std::cout << "the board: " << the_board << std::endl;
      assert(other_piece_type != NONE);
      AddPossibleMove(moves,start_row,start_column,moveto_row,moveto_column,color,CAPTURE,other_piece_type);
#ifdef DEBUG_ENGINE
      std::cout << "[EvalMoveTo] move results in capture" << std::endl;
#endif
      return CAPTURE;
    }
    // piece cannot be captured...
#ifdef DEBUG_ENGINE
    std::cout << "[EvalMoveTo] move would cause capture, but capture is not valid for this move"
	      << std::endl;
#endif
    return SQUARE_BLOCKED;
  }

  
  // square in question is free...

  if (must_capture) {
    // this move should have resulted in a capture...
#ifdef DEBUG_ENGINE
    std::cout << "[EvalMoveTo] must-capture is true, but there is no piece to capture at the move dest."
	      << std::endl;
#endif
    return INVALID_INDEX; // oops! no piece to capture...      
  }

  
  // square is free; no constraints on said move...

  AddPossibleMove(moves,start_row,start_column,moveto_row,moveto_column,color,SIMPLE_MOVE);
  
#ifdef DEBUG_ENGINE
  std::cout << "[EvalMoveTo] simple-move succeeds" << std::endl;
#endif

  
  return SIMPLE_MOVE;
}

// helper method for 'Diagonal'/'HorizVert' methods that follow:

enum { CHECKS_ON_RIGHT_DIAGONAL, CHECKS_ON_LEFT_DIAGONAL, CHECKS_ON_COLUMN, CHECKS_ON_ROW };

bool Piece::MovesDhvHelper( std::vector<Move> *moves, int checks_direction, Board &the_board, int color, int row, int column, int end_row, int end_column ) {
  int move_outcome = EvalMoveTo(moves,the_board,color,row,column,end_row,end_column);
  if (move_outcome == INVALID_INDEX) throw std::runtime_error("invalid move?");
  // quit 1st time we run into some other piece, same color...
  if (move_outcome == SQUARE_BLOCKED) return true;
  bool set_check = false;
  switch(checks_direction) {
    case CHECKS_ON_RIGHT_DIAGONAL:  set_check = ChecksOnRightDiagonal(the_board,color,end_row,end_column); break;
    case CHECKS_ON_LEFT_DIAGONAL:   set_check = ChecksOnLeftDiagonal(the_board,color,end_row,end_column);  break;
    case CHECKS_ON_COLUMN:          set_check = ChecksOnColumn(the_board,color,row,end_column);            break;
    case CHECKS_ON_ROW:             set_check = ChecksOnRow(the_board,color,end_row,column);               break;
    default: throw std::runtime_error("check direction?"); break;
  }
  if (set_check)
    moves->back().SetCheck();
  
  if (move_outcome == CAPTURE) return true;
  return false;
}

// bishop and queen can move diagonally (king too, but will 'special case' the king...

void Piece::MovesDiagonal( std::vector<Move> *moves, Board &the_board, int color, int row, int column) {
#ifdef DEBUG_ENGINE
  std::cout << "[MovesDiagonal] starting row/column: " << row << "/" << column << std::endl;
#endif
  
#ifdef DEBUG_ENGINE
  std::cout << "  move diagonally upwards, to left..." << std::endl;
#endif
  bool block_or_capture = false;
  for (int rc = row+1, cc = column-1; (rc < 8) && (cc >= 0) && !block_or_capture; rc++, cc--) {
    block_or_capture = MovesDhvHelper(moves,CHECKS_ON_RIGHT_DIAGONAL,the_board,color,row,column,rc,cc);
  }
  
#ifdef DEBUG_ENGINE
  std::cout << "  move diagonally upwards, to right..." << std::endl;
#endif
  block_or_capture = false;
  for (int rc = row+1, cc = column+1; (rc < 8) && (cc < 8) && !block_or_capture; rc++, cc++) {
    block_or_capture = MovesDhvHelper(moves,CHECKS_ON_LEFT_DIAGONAL,the_board,color,row,column,rc,cc);
  }

#ifdef DEBUG_ENGINE
  std::cout << "  move diagonally downwards, to left..." << std::endl;
#endif
  block_or_capture = false;
  for (int rc = row-1, cc = column-1; (rc >= 0) && (cc >= 0) && !block_or_capture; rc--, cc--) {
    block_or_capture = MovesDhvHelper(moves,CHECKS_ON_LEFT_DIAGONAL,the_board,color,row,column,rc,cc);
  }

#ifdef DEBUG_ENGINE
  std::cout << "  move diagonally downwards, to right..." << std::endl;
#endif
  block_or_capture = false;
  for (int rc = row-1, cc = column+1; (rc >= 0) && (cc < 8) && !block_or_capture; rc--, cc++) {
    block_or_capture = MovesDhvHelper(moves,CHECKS_ON_RIGHT_DIAGONAL,the_board,color,row,column,rc,cc);
  }

#ifdef DEBUG_ENGINE
  std::cout << "[MovesDiagonal]" << std::endl;
#endif
}

bool Piece::ChecksDiagonal(Board &the_board,int kings_row,int kings_column,int color,int row,int column) {
  int piece_type,piece_color;
     
  //  look for check, diagonally upwards, to left...

  for (int rc = row+1, cc = column-1; (rc < 8) && (cc >= 0); rc++, cc--) {
     if ( (rc == kings_row) && (cc == kings_column) ) return true;  // did we reach the king? if so, then check!
     if (the_board.GetPiece(piece_type,color,rc,cc)) break;         // if we reached any other piece then can quit
  }
  
  // look for check, diagonally upwards, to right...
  
  for (int rc = row+1, cc = column+1; (rc < 8) && (cc < 8); rc++, cc++) {
    if ( (rc == kings_row) && (cc == kings_column) ) return true;
    if (the_board.GetPiece(piece_type,color,rc,cc)) break;
  }

  // look for check, diagonally downwards, to left...

  for (int rc = row-1, cc = column-1; (rc >= 0) && (cc >= 0); rc--, cc--) {
    if ( (rc == kings_row) && (cc == kings_column) ) return true;
    if (the_board.GetPiece(piece_type,color,rc,cc)) break;
  }

  // look for check, diagonally downwards, to right..." << std::endl;

  for (int rc = row-1, cc = column+1; (rc >= 0) && (cc < 8); rc--, cc++) {
    if ( (rc == kings_row) && (cc == kings_column) ) return true;
    if (the_board.GetPiece(piece_type,color,rc,cc)) break;
  }

  return false;
}

// rook and queen can move horizontally or vertically...

void Piece::MovesHorizVert(std::vector<Move> *moves, Board &the_board, int color, int row, int column) {
  // moves horizontally to left from current position...
  bool block_or_capture = false;
  for (int rc = column - 1; (rc >= 0) && !block_or_capture; rc--) {
    block_or_capture = MovesDhvHelper(moves,CHECKS_ON_COLUMN,the_board,color,row,column,row,rc);
  }
  // moves horizontally to right from current position...
  block_or_capture = false;
  for (int rc = column + 1; (rc < 8) && !block_or_capture; rc++) {
    block_or_capture = MovesDhvHelper(moves,CHECKS_ON_COLUMN,the_board,color,row,column,row,rc);
  }
  // moves from current position, decreasing row...
  block_or_capture = false;
  for (int rr = row - 1; (rr >= 0) && !block_or_capture; rr--) {
    block_or_capture = MovesDhvHelper(moves,CHECKS_ON_ROW,the_board,color,row,column,rr,column);
  }
  
  // moves from current position, increasing row...
  block_or_capture = false;
  for (int rr = row + 1; (rr < 8) && !block_or_capture; rr++) {
    block_or_capture = MovesDhvHelper(moves,CHECKS_ON_ROW,the_board,color,row,column,rr,column);
  }
}

bool Piece::ChecksHorizVert(Board &the_board,int kings_row,int kings_column,int color,int row,int column) {
  int piece_type,piece_color;
     
  // look for check, horizontally to left from current position...
  for (int cc = column - 1; cc >= 0; cc--) {
    if ( (row == kings_row) && (cc == kings_column) ) return true;
     if (the_board.GetPiece(piece_type,color,row,cc)) break;
  }
  // look for check, horizontally to right from current position...
  for (int cc = column + 1; cc < 8; cc++) {
     if ( (row == kings_row) && (cc == kings_column) ) return true;
     if (the_board.GetPiece(piece_type,color,row,cc)) break;
  }
  // look for check, up from current position...
  for (int rr = row - 1; rr >= 0; rr--) {
     if ( (rr == kings_row) && (column == kings_column) ) return true;
     if (the_board.GetPiece(piece_type,color,rr,column)) break;
  }
  // look for check, down from current position...
  for (int rr = row + 1; rr < 8; rr++) {
     if ( (rr == kings_row) && (column == kings_column) ) return true;
     if (the_board.GetPiece(piece_type,color,rr,column)) break;
  }
  
  return false;
}

//***********************************************************************************************
// does some board position check the opposing players king?
//***********************************************************************************************

bool Piece::KingsPosition(bool &blocked, Board &the_board, int color, int row, int column) {
 int other_piece_type,other_piece_color;

 bool some_other_piece = the_board.GetPiece(other_piece_type,other_piece_color,row,column);

 // no piece on this square?...
 
 if (!some_other_piece)
   return false;

 // our piece?...
 
 if (other_piece_color == color) {
   blocked = true;
   return false;
 }
 
 // is it the (opponents) king?...
   
 return (other_piece_type == KING);
}

// within a column, is the opposing players king in check?...
//  (used by rook and queen)

bool Piece::ChecksOnColumn(Board &the_board, int color, int row, int column) {
  // look for unblocked access to king on a column...
  bool is_check = false;
  bool is_blocked = false;
  for (auto rr = row - 1; (rr >= 0) && !is_check && !is_blocked; rr--) {
     is_check = KingsPosition(is_blocked,the_board,color,rr,column);
  }
  is_blocked = false;
  for (auto rr = row + 1; (rr < 8) && !is_check && !is_blocked; rr++) {
     is_check = KingsPosition(is_blocked,the_board,color,rr,column);
  }
  return is_check;
}

// within a row, is the opposing players king in check?...
//  (used by rook and queen)

bool Piece::ChecksOnRow(Board &the_board, int color, int row, int column) {
  // look for unblocked access to king on a row...
  bool is_check = false;
  bool is_blocked = false;
  for (auto rc = column - 1; (rc >= 0) && !is_check && !is_blocked; rc--) {
     is_check = KingsPosition(is_blocked,the_board,color,row,rc);
  }
  is_blocked = false;
  for (auto rc = column + 1; (rc < 8) && !is_check && !is_blocked; rc++) {
     is_check = KingsPosition(is_blocked,the_board,color,row,rc);
  }
  return is_check;
}

// does bishop check king somewhere on 'right' diagonal?
// (used by bishop and queen)

bool Piece::ChecksOnRightDiagonal(Board &the_board,int color,int row,int column) {
  // look for unblocked access to king on right diagonal...
  bool is_check = false;
  bool is_blocked = false;
  for (int rr = row + 1, cc = column + 1; (rr < 8) && (cc < 8) && !is_check && !is_blocked; rr++, cc++) {
    is_check = KingsPosition(is_blocked,the_board,color,rr,column);
  }
  is_blocked = false;
  for (int rr = row - 1, cc = column - 1; (rr >= 0) && (cc >= 0) && !is_check && !is_blocked; rr--, cc--) {
    is_check = KingsPosition(is_blocked,the_board,color,rr,column);
  }
  return is_check;
}

// does bishop check king somewhere on 'left' diagonal?
// (used by bishop and queen)

bool Piece::ChecksOnLeftDiagonal(Board &the_board,int color,int row,int column) {
  // look for unblocked access to king on right diagonal...
  bool is_check = false;
  bool is_blocked = false;
  for (int rr = row + 1, cc = column - 1; (rr < 8) && (cc >= 0) && !is_check && !is_blocked; rr--, cc--) {
    is_check = KingsPosition(is_blocked,the_board,color,rr,column);
  }
  is_blocked = false;
  for (int rr = row - 1, cc = column + 1; (rr >= 0) && (cc < 8) && !is_check && !is_blocked; rr++, cc++) {
    is_check = KingsPosition(is_blocked,the_board,color,rr,column);
  }
  return is_check;
}

}
