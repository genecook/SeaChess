#ifndef __PIECES__

#include <string>
#include <vector>

namespace SeaChess {

//*****************************************************************************
// chess piece, pieces...
//*****************************************************************************

class Board;

class Piece {
  
public:
  Piece() {};
  virtual ~Piece() {};

  virtual int Type() { return NONE; };
  
  virtual std::string Name() { return "?"; };
  
  void AddPossibleMove(std::vector<Move> *moves, int start_row, int start_column, int end_row, int end_column, int color,
                       int outcome = UNKNOWN, int capture_type = NONE);

  // Moves - for a specified piece/board-position, what valid moves are possible?

  void SetSpeculativeMode(bool _speculative_mode=false ) { speculative_mode = _speculative_mode; };
  bool SpeculativeMode() { return speculative_mode; };
  
  virtual void Moves(std::vector<Move> *moves, Board &the_board, int color, int row, int column) {};

  virtual std::string Icon() { return "?"; };

  virtual bool Check(Board &the_board, int kings_row, int kings_column, int color, int row, int column) { return false; };

protected:  
  int EvalMoveTo(std::vector<Move> *moves, Board &the_board, int color, int current_row, int current_column, int moveto_row,
		 int moveto_column, bool can_capture = true, bool must_capture = false, bool en_passant = false);

  // bishop and queen can move diagonally (king too, but will 'special case' the king)...
  
  void MovesDiagonal(std::vector<Move> *moves, Board &the_board, int color, int row, int column);
  bool ChecksDiagonal(Board &the_board,int kings_row,int kings_column,int color,int row,int column);
  
  // rook and queen can move horizontally or vertically...
  
  void MovesHorizVert(std::vector<Move> *moves, Board &the_board, int color, int row, int column);
  bool ChecksHorizVert(Board &the_board,int kings_row,int kings_column,int color,int row,int column);

  // helper method for 'Diagonal'/'HorizVert' methods above:
  bool MovesDhvHelper(std::vector<Move> *moves, int checks_direction, Board &the_board, int color, int row, int column, int end_row, int end_column);

  bool KingsPosition(bool &blocked, Board &the_board, int color, int row, int column);

  bool ChecksOnColumn(Board &the_board, int color, int row, int column);
  bool ChecksOnRow(Board &the_board, int color, int row, int column);
  bool ChecksOnRightDiagonal(Board &the_board,int color,int row,int column);
  bool ChecksOnLeftDiagonal(Board &the_board,int color,int row,int column);

  friend std::ostream& operator<< (std::ostream &os, Piece &fld);
  
private:
  bool speculative_mode;
};

//***************************************************************************************
//***************************************************************************************

class Pawn : public Piece {
public:
  Pawn() {};
  int Type() { return PAWN; };
  std::string Name() { return "pawn"; };
  void Moves(std::vector<Move> *moves, Board &the_board, int color, int board_row, int board_column);
  std::string Icon() { return "P"; };
  bool Check(Board &the_board, int kings_row, int kings_column, int color, int row, int column);
private:
  bool CheckOnPromotion(Board &the_board, int row, int column, int color);
  void EvalBlocksCastling(std::vector<Move> *moves, Board &the_board, int color, int row, int column);
};

class Rook : public Piece {
public:
  Rook() {};
  int Type() { return ROOK; };
  std::string Name() { return "rook"; };
  void Moves(std::vector<Move> *moves, Board &the_board, int color, int board_row, int board_column);
  std::string Icon() { return "R"; };
  bool Check(Board &the_board, int kings_row, int kings_column, int color, int row, int column);  
};

class Knight : public Piece {
public:
  Knight() {};
  int Type() { return KNIGHT; };
  std::string Name() { return "knight"; };
  void Moves(std::vector<Move> *moves, Board &the_board, int color, int board_row, int board_column);
  std::string Icon() { return "N"; };
  bool Check(Board &the_board, int kings_row, int kings_column, int color, int row, int column);  
};

class Bishop : public Piece {
public:
  Bishop() {};
  int Type() { return BISHOP; };
  std::string Name() { return "bishop"; };
  void Moves(std::vector<Move> *moves, Board &the_board, int color, int board_row, int board_column);
  std::string Icon() { return "B"; };
  bool Check(Board &the_board, int kings_row, int kings_column, int color, int row, int column);  
};

class King : public Piece {
public:
  King() {};
  int Type() { return KING; };
  std::string Name() { return "king"; };
  void Moves(std::vector<Move> *moves, Board &the_board, int color, int board_row, int board_column);
  std::string Icon() { return "K"; };
  bool Check(Board &the_board, int kings_row, int kings_column, int color, int row, int column);
private:
  bool MoveCausesCheck(Board &the_board, int kings_row, int kings_column, int color);

  int opponents_king_row;
  int opponents_king_column;

  enum { NUM_MOVES=8 };
  const int rows[NUM_MOVES] = { 1,  1,  0, -1, -1, -1,  0,  1 };
  const int cols[NUM_MOVES] = { 0,  1,  1,  1,  0, -1, -1, -1 };
};

class Queen : public Piece {
public:
  Queen() {};
  int Type() { return QUEEN; };
  std::string Name() { return "queen"; };
  void Moves(std::vector<Move> *moves, Board &the_board, int color, int board_row, int board_column);
  std::string Icon() { return "Q"; };
  bool Check(Board &the_board, int kings_row, int kings_column, int color, int row, int column);  
};


//*****************************************************************************
// Pieces - just a convenient collection of chess pieces...
//*****************************************************************************

class Pieces {
public:
  Pieces() {};

  void GetMoves(std::vector<Move> *moves, Board &game_board, int type, int color, int row, int column) {
    switch(type) {
      case PAWN:   pawn.Moves(moves,game_board,color,row,column); break;
      case ROOK:   rook.Moves(moves,game_board,color,row,column); break;
      case KNIGHT: knight.Moves(moves,game_board,color,row,column); break;
      case BISHOP: bishop.Moves(moves,game_board,color,row,column); break;
      case QUEEN:  queen.Moves(moves,game_board,color,row,column); break;
      case KING:   king.Moves(moves,game_board,color,row,column); break;
	
      default:     throw std::runtime_error("Invalid chess piece type?");
                   break;
    }
  };
  
  bool Check(Board &game_board, int kings_row, int kings_column, int type, int color, int row, int column) {
    bool in_check = false;
    switch(type) {
      case PAWN:   in_check = pawn.Check(game_board,kings_row,kings_column,color,row,column); break;
      case ROOK:   in_check = rook.Check(game_board,kings_row,kings_column,color,row,column); break;
      case KNIGHT: in_check = knight.Check(game_board,kings_row,kings_column,color,row,column); break;
      case BISHOP: in_check = bishop.Check(game_board,kings_row,kings_column,color,row,column); break;
      case QUEEN:  in_check = queen.Check(game_board,kings_row,kings_column,color,row,column); break;
      case KING:   in_check = king.Check(game_board,kings_row,kings_column,color,row,column); break;
	
      default:     throw std::runtime_error("Invalid chess piece type?");
                   break;
    }
    return in_check;
  };
  
private:
  Pawn   pawn;
  Rook   rook;
  Knight knight;
  Bishop bishop;
  Queen  queen;
  King   king;
};

};

#endif
#define __PIECES__




