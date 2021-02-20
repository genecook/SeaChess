#include <string>
#include <stdexcept>
#include <unistd.h>
#include <iostream>
#include <sstream>
#include <stdlib.h>
#include <algorithm>
#include <bits/stdc++.h>
#include <chess.h>

namespace SeaChess {

#ifdef GRAPH_SUPPORT
extern int master_move_id;
#endif
  
//***********************************************************************************************
// build up tree of moves; pick the best one. minimax...
//***********************************************************************************************

int MovesTreeMinimax::ChooseMove(Move *next_move, Board &game_board, Move *suggested_move) {
  eval_count = 0;
  
#ifdef GRAPH_SUPPORT
  master_move_id = 0;
#endif
  
  ChooseMoveInner(root_node,game_board,Color(),MaxLevels(),INT_MIN,INT_MAX);
  PickBestMove(root_node,game_board,suggested_move);

  next_move->Set((Move *) root_node);

  //GraphMovesToFile("moves", root_node);

  return eval_count; // return total # of moves evaluated
}

 
//***********************************************************************************************
// build up tree of moves; pick the best one. minimax...
//***********************************************************************************************

void MovesTreeMinimax::ChooseMoveInner(MovesTreeNode *current_node, Board &current_board,
	         		       int current_color, int current_level, int alpha, int beta) {
  
  eval_count++; // keep track of total # of moves evaluated
  
  if (current_level == 0) {
    EvalBoard(current_node,current_board); // evaluate leaf node only
    return;  
  }
  
  // amend the current node with all possible moves for the current board/color...
  bool in_check = GetMoves(current_node,current_board,current_color,true,true);

  // no moves to be made? -- then its checkmate or a draw...
  if (current_node->PossibleMovesCount() == 0) {
    EvalBoard(current_node,current_board,in_check ? CHECKMATE : DRAW);
    current_node->SetOutcome(in_check ? CHECKMATE : DRAW);
    return;
  }
  
  // recursive descent for each possible move, for N levels...
  
  bool maximize_score = current_color == Color();
  int best_subtree_score = maximize_score ? -1000000 : 1000000;
  
  for (auto i = 0; i < current_node->PossibleMovesCount(); i++) {
     MovesTreeNode *pm = current_node->PossibleMove(i);
     Board updated_board = MakeMove(current_board,pm); 
     ChooseMoveInner(pm,updated_board,NextColor(current_color),current_level - 1,alpha,beta);
     // look for 'best' score --
     //   * maximize score for 'our' player - select move thaty maximizes score
     //   * minimize score for opponent - select move that minimizes impact of opponents move
     if (maximize_score) {
       if (pm->Score() > best_subtree_score) best_subtree_score = pm->Score();
       if (best_subtree_score > alpha)
	 alpha = best_subtree_score;
       if (alpha > beta)
       	 break;
     } else { 
       if (pm->Score() < best_subtree_score) best_subtree_score = pm->Score();
       if (best_subtree_score < beta)
	 beta = best_subtree_score;
       if (beta < alpha)
       	 break;
     }
  }

  // set this nodes score to the best sub-tree score...
  current_node->SetScore(best_subtree_score);
  
  if (current_level == MaxLevels()) {
    // leave top level moves in place, for best-move analysis...
  } else {
    // we're thru with this sub-node. flush it to conserve memory...
    current_node->Flush();
  }
}

}
