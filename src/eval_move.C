#include <string>
#include <stdexcept>
#include <unistd.h>
#include <iostream>
#include <stdlib.h>

#include <chess.h>

namespace SeaChess {
  
//*****************************************************************************************
// assign weighting based on outcome of some move...
//
// score move,as follows:
//
//       outcome            score
//   ----------------      -------
//   forced checkmate        10000
//   forced draw              2000
//   forced check             1000
//   check                      10
//   queen captured            200
//   bishop    "                90
//   rook      "               100
//   knight    "                75
//   pawn      "                20
//   pawn promotion*           200
//   simple-move                 1
//
// *assumed to queen
//*****************************************************************************************

int MovesTree::ScoreMove(MovesTreeNode *move, Board &current_board, int forced_score) {
  // 'bias' move based on which side's move is being evaluated...

  int bias = move->Color() != Color() ? -1 : 1;

  // after a move has been made, set score based on the outcome of the move...

  int score = 1; // token score: simple move
  
  if (forced_score == CHECKMATE)
    score = 10000;
  else if (forced_score == DRAW)
    score = 2000;
  else if (forced_score == CHECK)
    score = 1000;
  else if (move->Check())
    score = 10;
  else if (move->Outcome() == CAPTURE)
    switch(move->CaptureType()) {
      case QUEEN:  score = 200; break;
      case BISHOP: score = 90;  break;
      case ROOK:   score = 100; break;
      case KNIGHT: score = 75;  break;
      case PAWN:   score = 20;  break;
      default: break;
    }
  else if (move->Outcome() == PROMOTION)
    score = 200; 

  return score * bias;
}

void MovesTree::EvalMove(MovesTreeNode *move, Board &current_board, int forced_score) {
  move->SetScore(ScoreMove(move,current_board,forced_score));
}

/* simple-minded move evaluation from shannon...
f(p) = 200(K-K')
       + 9(Q-Q')
       + 5(R-R')
       + 3(B-B' + N-N')
       + 1(P-P')
       - 0.5(D-D' + S-S' + I-I')
       + 0.1(M-M') + ...
*/
  
void MovesTree::EvalBoard(MovesTreeNode *move, Board &current_board, int forced_score) {
  switch(forced_score) {
    case CHECKMATE: move->SetScore(10000); return; break;
    case DRAW:      move->SetScore(2000);  return; break;
    case CHECK:     move->SetScore(1000);  return; break;
    default: break;
  }
  
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
	}
     }
  }

  assert(this_side.kings == other_side.kings);
  
  int score = + 9 * (this_side.queens - other_side.queens)
              + 5 * (this_side.rooks - other_side.rooks)
              + 3 * ((this_side.bishops - other_side.bishops) + (this_side.knights - other_side.knights) )
              + 1 * (this_side.pawns - other_side.pawns);

  move->SetScore(score);
}
  
}



