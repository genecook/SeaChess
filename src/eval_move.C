#include <string>
#include <stdexcept>
#include <unistd.h>
#include <iostream>
#include <stdlib.h>

#include <chess.h>

namespace SeaChess {

//*********************************************************************
// piece-square tables - used to add bonuses for good piece placement
//
// see https://www.chessprogramming.org/Simplified_Evaluation_Function
//*********************************************************************

int pawns_table[8][8] = { {  0,  0,  0,  0,  0,  0,  0,  0 },
                          { 50, 50, 50, 50, 50, 50, 50, 50 },
                          { 10, 10, 20, 30, 30, 20, 10, 10 },
                          {  5,  5, 10, 25, 25, 10,  5,  5 },
                          {  0,  0,  0, 20, 20,  0,  0,  0 },
                          {  5, -5,-10,  0,  0,-10, -5,  5 },
                          {  5, 10, 10,-20,-20, 10, 10,  5 },
                          {  0,  0,  0,  0,  0,  0,  0,  0 } };
  
int knights_table[8][8] = { { -50,-40,-30,-30,-30,-30,-40,-50 },
                            { -40,-20,  0,  0,  0,  0,-20,-40 },
                            { -30,  0, 10, 15, 15, 10,  0,-30 },
                            { -30,  5, 15, 20, 20, 15,  5,-30 },
                            { -30,  0, 15, 20, 20, 15,  0,-30 },
                            { -30,  5, 10, 15, 15, 10,  5,-30 },
                            { -40,-20,  0,  5,  5,  0,-20,-40 },
                            { -50,-40,-30,-30,-30,-30,-40,-50 } };

int bishops_table[8][8] = { { -20,-10,-10,-10,-10,-10,-10,-20 },
                            { -10,  0,  0,  0,  0,  0,  0,-10 },
                            { -10,  0,  5, 10, 10,  5,  0,-10 },
                            { -10,  5,  5, 10, 10,  5,  5,-10 },
                            { -10,  0, 10, 10, 10, 10,  0,-10 },
                            { -10, 10, 10, 10, 10, 10, 10,-10 },
                            { -10,  5,  0,  0,  0,  0,  5,-10 },
                            { -20,-10,-10,-10,-10,-10,-10,-20 } };
  
int rooks_table[8][8] = { {  0,  0,  0,  0,  0,  0,  0,  0 },
                          {  5, 10, 10, 10, 10, 10, 10,  5 },
                          { -5,  0,  0,  0,  0,  0,  0, -5 },
                          { -5,  0,  0,  0,  0,  0,  0, -5 },
                          { -5,  0,  0,  0,  0,  0,  0, -5 },
                          { -5,  0,  0,  0,  0,  0,  0, -5 },
                          { -5,  0,  0,  0,  0,  0,  0, -5 },
                          {  0,  0,  0,  5,  5,  0,  0,  0 } };
  
int queens_table[8][8] = { { -20,-10,-10, -5, -5,-10,-10,-20 },
                           { -10,  0,  0,  0,  0,  0,  0,-10 },
                           { -10,  0,  5,  5,  5,  5,  0,-10 },
                           {  -5,  0,  5,  5,  5,  5,  0, -5 },
                           {   0,  0,  5,  5,  5,  5,  0, -5 },
                           { -10,  5,  5,  5,  5,  5,  0,-10 },
                           { -10,  0,  5,  0,  0,  0,  0,-10 },
                           { -20,-10,-10, -5, -5,-10,-10,-20 } };
  
int kings_table[8][8] = { { -30,-40,-40,-50,-50,-40,-40,-30 },
                          { -30,-40,-40,-50,-50,-40,-40,-30 },
                          { -30,-40,-40,-50,-50,-40,-40,-30 },
                          { -30,-40,-40,-50,-50,-40,-40,-30 },
                          { -20,-30,-30,-40,-40,-30,-30,-20 },
                          { -10,-20,-20,-20,-20,-20,-20,-10 },
                          {  20, 20,  0,  0,  0,  0, 20, 20 },
                          {  20, 30, 10,  0,  0, 10, 30, 20 } };

//*********************************************************************
// simple-minded move evaluation from shannon...
// f(p) = 200(K-K')
//       + 9(Q-Q')
//       + 5(R-R')
//       + 3(B-B' + N-N')
//       + 1(P-P')
//       - 0.5(D-D' + S-S' + I-I')
//       + 0.1(M-M') + ...
//*********************************************************************

int MovesTree::MaterialScore(Board &current_board) {
  // 'bias' move based on which side's move is being evaluated...

  struct piece_counts {
    piece_counts() : kings(0),queens(0),rooks(0),bishops(0),knights(0),pawns(0) {};
    int kings;
    int queens;
    int rooks;
    int bishops;
    int knights;
    int pawns;
  };

  piece_counts this_side;
  piece_counts other_side;

  int piece_placement_bonus = 0;
  
  for (int i = 0; i < 8; i++) {
     for (int j = 0; j < 8; j++) {
        int piece_type, piece_color;
        if (current_board.GetPiece(piece_type,piece_color,i,j)) {
	  piece_counts *side = (piece_color == Color()) ? &this_side : &other_side;
	  switch(piece_type) {
	    case KING:   side->kings++;   break;
	    case QUEEN:  side->queens++;  break;
	    case ROOK:   side->rooks++;   break;
	    case BISHOP: side->bishops++; break;
	    case KNIGHT: side->knights++; break;
	    case PAWN:   side->pawns++;  break;
	    default: break;
	  }
	  if (piece_color == Color()) {
	    switch(piece_type) {
	      case KING:   piece_placement_bonus += kings_table[i][j];   break;
	      case QUEEN:  piece_placement_bonus += queens_table[i][j];  break;
	      case ROOK:   piece_placement_bonus += rooks_table[i][j];   break;
	      case BISHOP: piece_placement_bonus += bishops_table[i][j]; break;
	      case KNIGHT: piece_placement_bonus += knights_table[i][j]; break;
	      case PAWN:   piece_placement_bonus += pawns_table[i][j];   break;
	      default: break;
	    }
	  }
	}
     }
  }

  assert(this_side.kings == other_side.kings);
  
  int score = + 900 * (this_side.queens - other_side.queens)
              + 500 * (this_side.rooks - other_side.rooks)
              + 300 * ((this_side.bishops - other_side.bishops) + (this_side.knights - other_side.knights) )
              + 100 * (this_side.pawns - other_side.pawns)
              + piece_placement_bonus;

  return score;
}

void MovesTree::EvalBoard(MovesTreeNode *move, Board &current_board, int forced_score) {
  // 'bias' score based on which side's move is being evaluated...

  int bias = move->Color() != Color() ? -1 : 1;
  
  switch(forced_score) {
    case CHECKMATE: move->SetScore(10000 * bias); return; break;
    case DRAW:      move->SetScore(2000 * bias);  return; break;
    case CHECK:     move->SetScore(1000 * bias);  return; break;
    default: break;
  }

  int score = MaterialScore(current_board) * bias;

  move->SetScore(score);
}
  
}



