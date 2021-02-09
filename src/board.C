#include <iostream>
#include <chess.h>

namespace SeaChess {
  
//******************************************************************************************
// setup board for new game...
//******************************************************************************************

void Board::Clear() {
  for (int i = 0; i < 8; i++) {
     for (int j = 0; j < 8; j++) {
        _board[i][j] = 0;  
     }
  }
}

void Board::Setup() {
  Clear();

  // load up board with white, black chess pieces...

  PlacePiece(0,0,ROOK,WHITE);    
  PlacePiece(0,1,KNIGHT,WHITE);
  PlacePiece(0,2,BISHOP,WHITE);
  PlacePiece(0,3,QUEEN,WHITE);
  PlacePiece(0,4,KING,WHITE);
  PlacePiece(0,5,BISHOP,WHITE);
  PlacePiece(0,6,KNIGHT,WHITE);
  PlacePiece(0,7,ROOK,WHITE);
  
  PlacePiece(1,0,PAWN,WHITE);
  PlacePiece(1,1,PAWN,WHITE);
  PlacePiece(1,2,PAWN,WHITE);
  PlacePiece(1,3,PAWN,WHITE);
  PlacePiece(1,4,PAWN,WHITE);
  PlacePiece(1,5,PAWN,WHITE);
  PlacePiece(1,6,PAWN,WHITE);
  PlacePiece(1,7,PAWN,WHITE);
  
  PlacePiece(7,0,ROOK,BLACK);
  PlacePiece(7,1,KNIGHT,BLACK);
  PlacePiece(7,2,BISHOP,BLACK);
  PlacePiece(7,3,QUEEN,BLACK);
  PlacePiece(7,4,KING,BLACK);
  PlacePiece(7,5,BISHOP,BLACK);
  PlacePiece(7,6,KNIGHT,BLACK);
  PlacePiece(7,7,ROOK,BLACK);

  PlacePiece(6,0,PAWN,BLACK);
  PlacePiece(6,1,PAWN,BLACK);
  PlacePiece(6,2,PAWN,BLACK);
  PlacePiece(6,3,PAWN,BLACK);
  PlacePiece(6,4,PAWN,BLACK);
  PlacePiece(6,5,PAWN,BLACK);
  PlacePiece(6,6,PAWN,BLACK);
  PlacePiece(6,7,PAWN,BLACK);
}

// translate algebraic coordinates (example: d2) into board row/column indices...

char column_chars[] = { 'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h' };

void Board::Index(int &row, int &column, std::string position) {
  int _row = -1,_column = -1;
  char rc;
  if (sscanf(position.c_str(),"%c%d",&rc,&_row) != 2)
    throw std::runtime_error("invalid position: " + position);
  _row -= 1;
  for (int i = 0; i < 8; i++) {
    if (column_chars[i] == rc) {
      _column = i;
      break;
    }
  }
  if ( !ValidRow(_row) || !ValidColumn(_column) )
    throw std::runtime_error("invalid position: " + position);
  row = _row;
  column = _column;
}

// translate row/column into algebraic coordinates...

std::string Board::Coordinates(int row,int column) {
  char tbuf[128];
  sprintf(tbuf,"%c%d",column_chars[column],row + 1);
  return tbuf;
}

// use this method to make a move, ie, update the board.
// special cases of castling, en passant picked off here
  
void Board::MakeMove(int start_row, int start_column, int end_row, int end_column) {
  int src_type,src_color;
    
  if (!GetPiece(src_type,src_color,start_row,start_column)) {
    std::cerr << "[Board::MakeMove] No piece to move at " << Coordinates(start_row,start_column) << std::endl;
    throw std::logic_error("No piece to move???");
  }

  bool do_castle  = false;      // is this a request to perform castling?
  bool kings_side = false;      // king side castling?

  bool en_passant = false;      // will pawn initial move setup case for en passant?

  bool en_passant_move = false; // does pawn capture via en passant?

  bool pawn_promotion = false;  // does pawn reach 8th row?

  if (src_type == KING) {
    // in this implementation, king move src/dest indicates castling request
    int castle_row = (src_color==WHITE) ? 0 : 7; 
    if ( (start_row==castle_row) && (start_column==4)
	   && (end_row==castle_row) && ((end_column==6) || (end_column==2)) ) {
      do_castle = true;
      kings_side = (end_column == 6); // white, kings side castle
    }
  } else if (src_type == PAWN) {
    // if pawn has moved two squares from its initial position,
    // then en passant is possible...
    if ( InitialPosition(start_row,start_column)
	 && ((end_row == (start_row + 2)) || (end_row == (start_row - 2))) ) {
      // only one pawn can be in 'en passant' state at a time
      SetEnPassant(end_row,end_column,src_color);
      en_passant = true;  // ANY other move will clear 'en passant' state
    } else if ( (end_row != start_row) && (end_column != start_column)
	      && !SquareOccupied(end_row,end_column)
	      && SquareOccupied(start_row,end_column)
	      && EnPassantSet(start_row,end_column,src_color) )  
      en_passant_move = true; // capture via en passant...
    else if ( ((end_row == 0) && (src_color == BLACK)) 
	      || ((end_row == 7) && (src_color == WHITE)) )
      pawn_promotion = true;      
  }
    
  if (do_castle) {
    // actual castling move handled elsewhere...
    if (!CastleValid(src_color,kings_side,true))
      throw std::logic_error("Invalid castle.");
  } else if (en_passant_move) {
    // pawn en passant move/capture...
    MakeMoveInner(start_row,start_column,end_row,end_column,start_row,end_column);
  } else {
    // NO castling, ie, normal move...
    MakeMoveInner(start_row,start_column,end_row,end_column);
  }
  
  if (pawn_promotion) {
    // only 'placement piece' supported thusfar is queen
    PlacePiece(end_row,end_column,QUEEN,src_color,0); // place the new queen; its NOT
                                                      //   an initial placement
  }

  if (!en_passant)
    ClearEnPassant();
}

// once a move is chosen and determined to be valid, this method is used
// to update the board...

void Board::MakeMoveInner(int start_row, int start_column, int end_row, int end_column,
			  int capture_row,int capture_column) {  
  
  // we assume the move has been validated, but nonetheless...
  if (!SquareOccupied(start_row,start_column)) {
    throw std::logic_error("Board::Move - no piece at specified row/column.");
  }
    
  // copy 'piece', clear 'initial position' bit...
  _board[end_row][end_column] = _board[start_row][start_column] & 0x7f;  
  _board[start_row][start_column] = 0;

  if (capture_row >= 0) {
    _board[capture_row][capture_column] = 0;   // en passant capture is only such case
  }
}

// is a board piece still in its initial position?...

bool Board::PieceHasMoved(int expected_color,int expected_type,int row,int column) {
  // no piece at the expected position?...
  int type,color;
  if (!GetPiece(type,color,row,column)) return true; 

  // not the piece we were expecting?...
  if ( (color != expected_color) || (type != expected_type) ) return true;

  // the piece has been moved?...
  if (!InitialPosition(row,column)) return true;

  // the expected piece is still in its initial position...
  return false;
}

// validate castling for a side; optionally make the required moves
// return final king column position if valid...

int Board::CastleValid(int color, bool kings_side, bool do_move) {
  int castle_row = (color == WHITE) ? 0 : 7;
  
  // can't castle if king or rook has moved, or there are pieces in the way...

  if (PieceHasMoved(color,KING,castle_row,4)) {// once king has moved castling is invalid
    return 0;
  }

  if ( kings_side && (PieceHasMoved(color,ROOK,castle_row,7) || SquareOccupied(castle_row,5) || SquareOccupied(castle_row,6)) ) {
    return 0;
  }
  
  if ( !kings_side && (PieceHasMoved(color,ROOK,castle_row,0) || SquareOccupied(castle_row,1) || SquareOccupied(castle_row,2) || SquareOccupied(castle_row,3)) ) {
    return 0;
  }

  if (do_move)
    MakeCastle(color,kings_side);

  int end_column = (kings_side) ? 6 : 2; // return column where king will be after successful castle

  return end_column;
}

// perform castling...
  
void Board::MakeCastle(int color, bool kings_side) {
  int castle_row = (color == WHITE) ? 0 : 7;
  
  if (kings_side) {
    MakeMoveInner(castle_row,4,castle_row,6);  // move the king,
    MakeMoveInner(castle_row,7,castle_row,5);  //   "   "  rook
  } else { // queens side
    MakeMoveInner(castle_row,4,castle_row,2);  // move the king
    MakeMoveInner(castle_row,0,castle_row,3);  //   "   "  rook
  }
}

// stream out chess board. called after each (engine) move has been made...

std::ostream& operator<< (std::ostream &os, Board &fld) {
  os << "#" << std::endl;
  for (int row = 7; row >= 0; row--) {
    os << "# " << row + 1 << "  " << fld.Display(row,0) << " " << fld.Display(row,1)
       << " " << fld.Display(row,2) << " " << fld.Display(row,3)
       << " " << fld.Display(row,4) << " " << fld.Display(row,5)
       << " " << fld.Display(row,6) << " " << fld.Display(row,7);
    os << "\n";
  }
  os << "\n#    a  b  c  d  e  f  g  h\n";
  return os;
}

void Board::Save(std::ofstream &saveFile) {
  // we assume saveFile to be an open binary output stream...
  unsigned char tbuf[1024];
  int index = 0;
  for (auto i = 0; i < 8; i++) {
     for (auto j = 0; j < 8; j++) {
        tbuf[index++] = _board[i][j];
     }
  }
  tbuf[index++] = en_passant_row;
  tbuf[index++] = en_passant_column;
  tbuf[index++]= en_passant_color;
  
  saveFile.write((char *) tbuf,67);
  
  if (!saveFile)
    throw std::runtime_error("Problems writing board state to file.");    
}

void Board::Load(std::ifstream &loadFile) {
  // we assume loadFile to be an open binary input stream...
  unsigned char tbuf[1024];
  
  loadFile.read((char *) tbuf,67);
  
  if (loadFile.gcount() != 67)
    throw std::runtime_error("Problems reading board state from file.");
  if (!loadFile)
    throw std::runtime_error("Problems reading board state from file.");    
  
  int index = 0;
  for (auto i = 0; i < 8; i++) {
     for (auto j = 0; j < 8; j++) {
        _board[i][j] = tbuf[index++];
     }
  }
  en_passant_row = tbuf[index++];
  en_passant_column = tbuf[index++];
  en_passant_color = tbuf[index++];
}

}


