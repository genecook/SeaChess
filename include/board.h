#ifndef __BOARD__

#include <string>
#include <iostream>
#include <fstream>

//***********************************************************************************
// the chess board...
//***********************************************************************************

namespace SeaChess {
  
class Board {
public:
  Board() : en_passant_row(0), en_passant_column(0), en_passant_color(0) { Clear(); };
  //~Board() {};

  void Clear();
  void Setup();
  
  static bool ValidRow(int row)       { return (row >= 0) && (row < 8); };
  static bool ValidColumn(int column) { return ValidRow(column); };

  static bool ValidPosition(int row, int column) {return ValidRow(row) && ValidColumn(column); };

  // translate board coordinates (example: d2) into board (array) index...

  static void Index(int &row, int &column, std::string position);
  static std::string Coordinates(int row,int column);

  // starting/ending row applies to pawns...
  
  static bool StartingRow(int row,int color) {
    return ( (color == WHITE) && (row == 1) ) || ( (color == BLACK) && (row == 6) );
  };

  static bool EndingRow(int row,int color) {
    return ( (color == WHITE) && (row == 7) ) || ( (color == BLACK) && (row == 0) );
  };

  // before a game starts, the pieces must of course be placed...
  
  void PlacePiece(int row, int column, int piece_type, int color, int init_position=0x80) {
    _board[row][column] = init_position | (color<<4) | piece_type;
  };

  // any square with non-zero value is occupied...
  
  bool SquareOccupied(int row, int column) {
    return (_board[row][column] != 0);
  };

  // top bit of square used to indicate piece is at initial placement...
  
  bool InitialPosition(int row, int column) {
    return ( (_board[row][column] & 0x80) == 0x80 );
  };

  // record pawn 'en passant' position/color...
  
  void SetEnPassant(int row, int column,int color) {
    en_passant_row    = row;
    en_passant_column = column;
    en_passant_color  = color;
  };

  // clear en passant state...
  
  void ClearEnPassant() {
    en_passant_row = en_passant_column = en_passant_color = 0;
  };

  // is 'en passant' state set?...
  
  bool EnPassantSet(int row, int column,int color) {
    return ( (en_passant_row == row) && (en_passant_column == column)
	     && (en_passant_color != color) ); // set on previous move, thus color should NOT match
  };

  // set type,color, return true for piece on a square, if there is one;
  // false otherwise...
  
  bool GetPiece(int &piece_type, int &color, int row, int column) {
    if (SquareOccupied(row,column)) {
      piece_type = _board[row][column] & 0xf;
      color      = (_board[row][column] >> 4) & 0x3;
      return true;
    }
    
    return false;
  };

  // is the king of specified color on a square?...
  
  bool KingByColor(int row, int column, int color) {
    if (!ValidRow(row) || !ValidColumn(column))
      return false;
    
    int ktype, kcolor;    
    if (!GetPiece(ktype,kcolor,row,column))
      return false;
    
    return (ktype == KING) && (kcolor == color);
  };

  // is the opposing king on a square?...
  
  bool OpposingKing(int row, int column, int color) {
    int opposing_color = (color==WHITE) ? BLACK : WHITE;
    return KingByColor(row,column,opposing_color);
  };

  // get piece of type and color...
  
  bool FindPiece(int &row,int &column,int ptype, int pcolor) {
    for (auto i = 0; i < 8; i++) {
      for (auto j = 0; j < 8; j++) {
        int ktype, kcolor;
        if (GetPiece(ktype,kcolor,i,j) && (ktype == ptype) && (kcolor == pcolor) ) {
          row = i;
	        column = j;
	        return true;
	      }
      }
    }

    return false;
  };

  bool PieceExists(int ptype, int pcolor) {
    int row, column;
    return FindPiece(row,column,ptype,pcolor);
  };

  // get king or die trying...

  void GetKing(int &row,int &column,int color) {
    // shortcut - in some cases we may already have the king...
    
    int ktype, kcolor;    
    if (ValidRow(row) && ValidColumn(column) && GetPiece(ktype,kcolor,row,column) && (ktype == KING) && (kcolor == color) )
      return;

    // no? go find the king...
    for (auto i = 0; i < 8; i++) {
       for (auto j = 0; j < 8; j++) {
          if (GetPiece(ktype,kcolor,i,j) && (ktype == KING) && (kcolor == color) ) {
            row = i;
	    column = j;
	    return;
	  }
       }
    }
    throw std::logic_error("Wheres the " + ColorAsStr(color) + "king???");    
  };

  int PieceCount(int color) {
    int pc = 0;
    int ptype, pcolor;
    for (auto i = 0; i < 8; i++)
       for (auto j = 0; j < 8; j++)
          if (GetPiece(ptype,pcolor,i,j) && (pcolor == color) ) pc++;

    return pc;
  };

  int TotalPieceCount() {
    return PieceCount(WHITE) + PieceCount(BLACK);
  };

  void GetOpposingKing(int &row,int &column,int color) {
    int opposing_color = (color==WHITE) ? BLACK : WHITE;
    GetKing(row,column,opposing_color);
  };
  
  // used when displaying the board to stdout...
  
  std::string Display(int row, int column) {
    std::string square = ". ";
    int type, color;
    if (GetPiece(type,color,row,column)) {
      square = ColorChar(color) + PieceIcon(type);
    }
    return square;
  };

  // use this method to make a move, ie, update the board.
  
  void MakeMove(int start_row, int start_column, int end_row, int end_column);

  // return true only if expected board piece is found, and the piece has not yet been moved...
  
  bool PieceHasMoved(int expected_color,int expected_type,int row,int column);
  
  // validate castling for a side; optionally make the required moves
  // return true if valid (and successful) castle...

  int CastleValid(int color, bool kings_side, bool do_move=false);  // validate castling
  void MakeCastle(int color, bool kings_side);                      // do the actual board updates
  
  // situations when castling is blocked must be detected when pieces are moved...
  //
  // if a piece covers (can be moved to) a square between the rook and king one one side or the
  // other than castling is blocked for that side...

  int EvalBlockCastling(int color,int row,int column)  { 
    int opposing_color = (color==WHITE) ? BLACK : WHITE;

    int kings_row = (opposing_color == WHITE) ? 0 : 7; // white kings (at start of game) is on row zero
                                                       // conversely, black king at start is on row seven
    if (row == kings_row) {
      if (column > 4 && column < 7) {
        //std::cout << "QUEEN SIDE CASTLING BLOCKED BY " << ColorAsStr(color) << "!" << std::endl;
        return opposing_color; 
      }
      if (column > 0 && column < 4) {
        //std::cout << "KINGS SIDE CASTLING BLOCKED BY " << ColorAsStr(color) << "!" << std::endl;
        return opposing_color; 
      }
    }

    return NOT_SET;
  }

  friend std::ostream& operator<< (std::ostream &os, Board &fld);

  // save or load board state to/from stream...
  
  void Save(std::ofstream &saveFile);
  void Load(std::ifstream &loadFile);
  
protected:
  void MakeMoveInner(int start_row, int start_column, int end_row, int end_column,
		     int capture_row = -1, int capture_column = -1);
private:
  unsigned char _board[8][8];        // the game board is 64 bytes

  unsigned char en_passant_row;      //
  unsigned char en_passant_column;   // only one pawn at a time can be in 
  unsigned char en_passant_color;    //   'en passant' state
};

};

#endif
#define __BOARD__




