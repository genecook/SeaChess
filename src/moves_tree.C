#include <string>
#include <stdexcept>
#include <unistd.h>
#include <iostream>
#include <stdlib.h>
#include <algorithm>

#include <chess.h>

namespace SeaChess {
  
//***********************************************************************************************
// build up tree of moves; pick the best one. minimax...
//***********************************************************************************************

int MovesTree::ChooseMove(Move *next_move, Board &game_board, Move *suggested_move) {
  eval_count = 0;
  
  ChooseMoveInner(root_node,game_board,Color(),0);
  PickBestMove(root_node,game_board,NULL /*suggested_move*/);

  next_move->Set((Move *) root_node);
		  
  return eval_count; // return total # of moves evaluated
}

 
//***********************************************************************************************
// build up tree of moves; pick the best one. minimax...
//***********************************************************************************************

void MovesTree::ChooseMoveInner(MovesTreeNode *current_node, Board &current_board, int current_color, int current_level) {
  
  EvalMove(current_node,current_board); // evaluate every move to yield raw score

  eval_count++;
  
  if (current_level == MaxLevels())
    return;  

  // amend the current node with all possible moves for the current board/color...
  
  bool in_check = GetMoves(current_node,current_board,current_color);

  // recursive descent for each possible move, for N levels...
  
  for (auto pvi = current_node->possible_moves.begin(); pvi != current_node->possible_moves.end(); pvi++) {
     Board updated_board = MakeMove(current_board,*pvi); 
     ChooseMoveInner(*pvi,updated_board,NextColor(current_color),current_level + 1);
  }
  
  // no moves to be made? -- then its checkmate or a draw...
  
  if (current_node->possible_moves.size() == 0) {
    EvalMove(current_node,current_board,in_check ? CHECKMATE : DRAW);
    current_node->SetOutcome(in_check ? CHECKMATE : DRAW);
    return;
  }

  // update sub-tree node scores from current score...

  int subtree_score = 0;
  bool init_subtree_score = true;

  bool use_positive_score = current_color == Color();
  
  for (auto pvi = current_node->possible_moves.begin(); pvi != current_node->possible_moves.end(); pvi++) {
     if (init_subtree_score) {
       subtree_score = (*pvi)->Score();
       init_subtree_score = false;
       continue;
     }

     // look for 'best' score --
     //   * positive score for 'our' player indicates the best score
     //   * negative score for opponents best score, and thus move to be avoided

     bool better_score = false;

     if (use_positive_score)
       better_score = (*pvi)->Score() > subtree_score;
     else 
       better_score = (*pvi)->Score() < subtree_score;
     
     if (better_score) {
       subtree_score = (*pvi)->Score();
     }
  }

  // augment this nodes raw score with the appropriate sub-tree score...

  current_node->SetScore(current_node->Score() + subtree_score);
}

//***********************************************************************************************
// build up list of moves possible for specified color, given a board state.
// return true if king is in check
//***********************************************************************************************

bool MovesTree::GetMoves(std::vector<Move> *possible_moves, Board &game_board, int color,bool avoid_check) {
  // pretty crude - scan the entire board each time to get current set of pieces for a side...

  kings_row = 0;
  kings_column = 0;
  
  game_board.GetKing(kings_row,kings_column,color);
  
  bool in_check = false;
  
  std::vector<Move> all_possible_moves;
  
  for (int i = 0; i < 8; i++) {
     for (int j = 0; j < 8; j++) {
        int piece_type, piece_color;
        if (game_board.GetPiece(piece_type, piece_color,i,j)) {
	  if (piece_color == color) {
	    // this is 'our' piece... 
	    pieces.GetMoves(&all_possible_moves,game_board,piece_type,piece_color,i,j);
          } else {
	    // for current board state, does 'opponents' piece have us in check?
	    in_check |= pieces.Check(game_board,kings_row,kings_column,piece_type,piece_color,i,j);
	  }
	}
     }
  }

  for (auto pmi = all_possible_moves.begin(); pmi != all_possible_moves.end(); pmi++) {
     if (avoid_check) {
       MovesTreeNode tn = *pmi;
       Board updated_board = MakeMove(game_board,&tn); 
       if (Check(updated_board,color)) {
         // ignore any move that places or leaves 'our' king in check...
	 continue;
       }
     }
     possible_moves->push_back(*pmi);
  }

  return in_check;
}

bool MovesTree::GetMoves(MovesTreeNode *node, Board &game_board, int color,bool avoid_check) {
  std::vector<Move> all_possible_moves;
  
  bool in_check = GetMoves(&all_possible_moves,game_board,color,avoid_check);
  
  for (auto pmi = all_possible_moves.begin(); pmi != all_possible_moves.end(); pmi++) {
     node->AddMove(*pmi);
  }
  
  return in_check;
}

//***********************************************************************************************
// count a sides pieces by type...
//***********************************************************************************************

void MovesTree::CountPieces(struct piece_counts &counts, Board &game_board,int color) {
  for (int i = 0; i < 8; i++) {
     for (int j = 0; j < 8; j++) {
        int piece_type, piece_color;
        if (game_board.GetPiece(piece_type, piece_color,i,j) && (piece_color == color)) {
          switch(piece_type) {
            case KING: counts.kings++; break;
            case QUEEN: counts.queens++; break;
            case BISHOP: counts.bishops++; break;
            case KNIGHT: counts.knights++; break;
            case ROOK: counts.rooks++; break;
            case PAWN: counts.pawns++; break;
            default: break;
          }
        }
     }
  }

  assert(counts.kings == 1); // sanity check: we do have a king, nes pa?
}

//***********************************************************************************************
// count the # of pieces a side has...
//***********************************************************************************************

int MovesTree::GetPieceCount(Move *node,Board &game_board,int color) {
  int piece_cnt = 0;

  for (int i = 0; i < 8; i++) {
     for (int j = 0; j < 8; j++) {
        int piece_type, piece_color;
        if (game_board.GetPiece(piece_type, piece_color,i,j) && (piece_color == color))
          piece_cnt++;
     }
  }

  return piece_cnt;
}

//***********************************************************************************************
// Check - given a board state, is the king of 'color' in check?...
//***********************************************************************************************

 bool MovesTree::Check(Board &board,int color) {
  board.GetKing(kings_row,kings_column,color);
  
  int opposing_color = (color == WHITE) ? BLACK : WHITE;
  
  bool in_check = false;
  
  for (int i = 0; (i < 8) && !in_check; i++) {
     for (int j = 0; (j < 8) && !in_check; j++) {
       int piece_type, piece_color;
       if ( board.GetPiece(piece_type, piece_color,i,j) && (piece_color == opposing_color) ) {
	        in_check |= pieces.Check(board,kings_row,kings_column,piece_type,piece_color,i,j);
       }
    }
  }

  return in_check;
}

//***************************************************************************************
// After all possible moves have been identified and evaluated, from the top level,
// for the current stage of the game, pick the best move...
//***************************************************************************************

bool MovesTree::BestScore(MovesTreeNode *this_move, MovesTreeNode *previous_move) {
  return ( this_move->Score() > previous_move->Score() );
}

void MovesTree::PickBestMove(MovesTreeNode *root_node, Board &game_board, Move *suggested_move) {
  bool have_moves = false; // true once we have a starting 'best' move 

  // is there a suggested move?...
  if ( (suggested_move != NULL) && suggested_move->Valid() ) {
    // yes. see if this move corresponds to one of the move choices...
    for (auto pvm = root_node->possible_moves.begin(); pvm != root_node->possible_moves.end(); pvm++) {
       if ( (*pvm)->Match(suggested_move) ) {
	 // it does. lets use the suggested move, and hope its a good one...
         root_node->Set(*pvm);
	 have_moves = true;
	 break;
       }
    }
  }

  if (have_moves) {
    return;
  }

  // look for the move with the best score...

  // at least in the current implementation, there will be cases where there are multiple
  // moves with the same score. lets randomize the list, both to make things more
  // interesting and to get more test coverage...
  
  std::random_shuffle( root_node->possible_moves.begin(), root_node->possible_moves.end() );

  //std::cout << "[PickBestMove] entered..." << std::endl;
  
  for (auto pvm = root_node->possible_moves.begin(); pvm != root_node->possible_moves.end(); pvm++) {
    //std::cout << "\t" << *(*pvm) << std::endl;

     if (!have_moves) {
       root_node->Set(*pvm);
       have_moves = true;
       continue;
     }
     
     if ( BestScore( *pvm, root_node ) ) {
         root_node->Set(*pvm);
         root_node->SetScore((*pvm)->Score());
     }
  }

  //std::cout << std::endl;
  
  if (have_moves) {
    // there were some valid moves to choose from, cool
  } else {
    // this is the root node. no (valid) moves can be made. will ASSUME draw or checkmate..."
    root_node->SetOutcome(RESIGN);
  }
}

//***********************************************************************************************
// make a move...
//***********************************************************************************************

#define MAKEMOVE_CHECKS

Board MovesTree::MakeMove(Board &board, MovesTreeNode *pv) {
#ifdef MAKEMOVE_CHECKS
  // validate move start/end coordinates...
  
  if ( !board.ValidRow(pv->StartRow()) || !board.ValidColumn(pv->StartColumn()) )
    std::runtime_error("MakeMove: Bad move start coordinate!");
  
  if ( !board.ValidRow(pv->EndRow()) || !board.ValidColumn(pv->EndColumn()) )
    std::runtime_error("MakeMove: Bad move end coordinate!");

  // validate the piece to be moved...
  
  int type, color;
  
  if ( !board.GetPiece(type,color,pv->StartRow(),pv->StartColumn()) )
    std::runtime_error("MakeMove: no piece at start location!");
#endif
  
  // "poor mans" move eval...

  bool capture = board.SquareOccupied(pv->EndRow(),pv->EndColumn());
		
  pv->SetOutcome( capture ? CAPTURE : SIMPLE_MOVE );

  // move the piece...

  Board updated_board = board;

  try {
    updated_board.MakeMove(pv->StartRow(),pv->StartColumn(),pv->EndRow(),pv->EndColumn());
  } catch(std::logic_error reason) {
    std::cout << "# Invalid move, reason: '" << reason.what() << std::endl;
    std::cerr << "updated board: " << updated_board << std::endl;
    exit(1);
  }

  return updated_board;
}

};
