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
//   forced checkmate      1000000
//   forced draw            100000
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
    score = 1000000;
  else if (forced_score == DRAW)
    score = 100000;
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
  // 'bias' move based on which side's move is being evaluated...

  int bias = move->Color() != Color() ? -1 : 1;

  // after a move has been made, set score based on the outcome of the move...

  if (forced_score == CHECKMATE)
    move->SetScore(1000000 * bias);
  else if (forced_score == DRAW)
    move->SetScore(100000 * bias);
  else if (forced_score == CHECK)
    move->SetScore(1000 * bias);
  else if (move->Check()) {
    move->SetScore(10 * bias);
  } else if (move->Outcome() == CAPTURE) {
    assert(move->CaptureType() != NONE);
    switch(move->CaptureType()) {
      case QUEEN:  move->SetScore(200 * bias); break;
      case BISHOP: move->SetScore( 90 * bias); break;
      case ROOK:   move->SetScore(100 * bias); break;
      case KNIGHT: move->SetScore( 75 * bias); break;
      case PAWN:   move->SetScore( 20 * bias); break;
      default:
	std::cout << "capture type: " << move->CaptureType() << std::endl;
	break;
    }
  } else if (move->Outcome() == PROMOTION) {
    move->SetScore(200 * bias); 
  } else {
    move->SetScore(1 * bias);
  }

  int check_score = ScoreMove(move,current_board,forced_score);

  if (check_score != move->Score()) {
    std::cout << "\t[EvalMove] board: " << current_board << " move: " << *move;
    std::cout << ", check-score/move-score: " << check_score << "/" << move->Score() << std::endl;
  }
  assert (check_score == move->Score());
}

}



