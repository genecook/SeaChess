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
int master_move_id;
#endif
  
//***********************************************************************************************
// build up tree of moves; pick the best one. minimax...
//***********************************************************************************************

int MovesTree::ChooseMove(Move *next_move, Board &game_board, Move *suggested_move) {
  eval_count = 0;
  
#ifdef GRAPH_SUPPORT
  master_move_id = 0;
#endif
  
  ChooseMoveInner(root_node,game_board,Color(),MaxLevels(),INT_MIN,INT_MAX);
  PickBestMove(root_node,game_board,NULL /*suggested_move*/);

  next_move->Set((Move *) root_node);

  GraphMovesToFile("moves", root_node);

  return eval_count; // return total # of moves evaluated
}

 
//***********************************************************************************************
// build up tree of moves; pick the best one. minimax...
//***********************************************************************************************

  void MovesTree::ChooseMoveInner(MovesTreeNode *current_node, Board &current_board,
				  int current_color, int current_level, int alpha, int beta) {
  
  eval_count++; // keep track of total # of moves evaluated
  
  if (current_level == 0) {
    EvalBoard(current_node,current_board); // evaluate leaf node only
    return;  
  }
  
  // amend the current node with all possible moves for the current board/color...
  bool in_check = GetMoves(current_node,current_board,current_color,true,true);

  // no moves to be made? -- then its checkmate or a draw...
  if (current_node->possible_moves.size() == 0) {
    EvalBoard(current_node,current_board,in_check ? CHECKMATE : DRAW);
    current_node->SetOutcome(in_check ? CHECKMATE : DRAW);
    return;
  }
  
  // recursive descent for each possible move, for N levels...
  
  bool maximize_score = current_color == Color();
  int best_subtree_score = maximize_score ? -1000000 : 1000000;
  
  for (auto pvi = current_node->possible_moves.begin(); pvi != current_node->possible_moves.end(); pvi++) {
     Board updated_board = MakeMove(current_board,*pvi); 
     ChooseMoveInner(*pvi,updated_board,NextColor(current_color),current_level - 1,alpha,beta);
     // look for 'best' score --
     //   * maximize score for 'our' player - select move thaty maximizes score
     //   * minimize score for opponent - select move that minimizes impact of opponents move
     if (maximize_score) {
       if ((*pvi)->Score() > best_subtree_score) best_subtree_score = (*pvi)->Score();
       if (best_subtree_score > alpha)
	 alpha = best_subtree_score;
       if (alpha > beta)
       	 break;
     } else { 
       if ((*pvi)->Score() < best_subtree_score) best_subtree_score = (*pvi)->Score();
       if (best_subtree_score < beta)
	 beta = best_subtree_score;
       if (beta < alpha)
       	 break;
     }
  }

  // set this nodes score to the best sub-tree score...
  current_node->SetScore(best_subtree_score);
}

//***********************************************************************************************
// build up list of moves possible for specified color, given a board state.
// return true if king is in check
//***********************************************************************************************

bool MovesTree::GetMoves(std::vector<Move> *possible_moves, Board &game_board, int color, bool avoid_check) {
  // pretty crude - scan the entire board each time to get current set of pieces for a side...

  // for current board state, does 'opponents' piece have us in check?

  kings_row = 0;
  kings_column = 0;
  
  game_board.GetKing(kings_row,kings_column,color);
  
  bool in_check = false;

  for (int i = 0; (i < 8) && !in_check; i++) {
     for (int j = 0; (j < 8) && !in_check; j++) {
        int piece_type, piece_color;
        if (game_board.GetPiece(piece_type,piece_color,i,j)) {
	  if (piece_color == color)
	    continue; // this is 'our' piece... 
	  // for current board state, does 'opponents' piece have us in check?
	  if (pieces.Check(game_board,kings_row,kings_column,piece_type,piece_color,i,j)) {
	    in_check = true;
	  }
	}
     }
  }

  bool castling_enabled = true;
  
  if (in_check) {
    // king is in check, so no need to check castling intermediate squares...
  } else {
    // make sure no opponents piece 'covers' any intermediate castling square...
    int squares_to_check[] = { 0,0,0,0 };
    if (game_board.CastleValid(color,/* kings-side */ true)) {
      // validate castling, kings side...
      squares_to_check[0] = 5;
      squares_to_check[1] = 6;
    } else if (game_board.CastleValid(color,/* queen-side */ false)) {
    // validate castling, queens side...
      squares_to_check[0] = 1;
      squares_to_check[1] = 2;
      squares_to_check[2] = 3;
    }
    if (squares_to_check[0] != 0)
      for (int i = 0; (i < 8) && castling_enabled; i++) {
         for (int j = 0; (j < 8) && castling_enabled; j++) {
            int piece_type, piece_color;
            if (game_board.GetPiece(piece_type, piece_color,i,j)) {
	      if (piece_color == color)
	        continue; // this is 'our' piece...
	      // for current board state, does 'opponents' piece 'cover' a 'castling' square?
	      for (int k = 0; (squares_to_check[k] != 0) && castling_enabled; k++)
	         if (pieces.Check(game_board,kings_row,squares_to_check[k],piece_type,piece_color,i,j)) {
	           castling_enabled = false;
	      }
	    }
         }
      }
  }
    
  std::vector<Move> all_possible_moves;
  
  for (int i = 0; i < 8; i++) {
     for (int j = 0; j < 8; j++) {
        int piece_type, piece_color;
        if (game_board.GetPiece(piece_type, piece_color,i,j)) {
	  if (piece_color == color) {
	    // this is 'our' piece... 
	    pieces.GetMoves(&all_possible_moves,game_board,piece_type,piece_color,i,j,in_check,castling_enabled);
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

bool movesortfunction(MovesTreeNode *m1, MovesTreeNode *m2) {
  return abs(m1->Score()) > abs(m2->Score());
}
  
bool MovesTree::GetMoves(MovesTreeNode *node, Board &game_board, int color,bool avoid_check, bool sort_moves) {
  std::vector<Move> all_possible_moves;
  
  bool in_check = GetMoves(&all_possible_moves,game_board,color,avoid_check);

  for (auto pmi = all_possible_moves.begin(); pmi != all_possible_moves.end(); pmi++) {
     node->AddMove(*pmi);
  }

  if (sort_moves) {
    for (auto pmi = node->possible_moves.begin(); pmi != node->possible_moves.end(); pmi++) {
       Board updated_board = MakeMove(game_board,*pmi); 
       EvalBoard(*pmi,updated_board); // evaluate every move to yield raw score
    }
    std::sort(node->possible_moves.begin(), node->possible_moves.end(), movesortfunction);
    // leave move scores in tact - ASSUME move scores will be overwritten 
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
  
  //std::random_shuffle( root_node->possible_moves.begin(), root_node->possible_moves.end() );

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

//***********************************************************************************************
// graph game tree to file...
//***********************************************************************************************

void MovesTree::GraphMovesToFile(const std::string &outfile, MovesTreeNode *node) {
#ifndef GRAPH_SUPPORT
    std::cout << "NOTE: This configuration does NOT support moves-tree graphing." << std::endl;
    return;
#else  
    int node_ID = -1;

    char tbuf[1024];
    sprintf(tbuf,"%s.dot",outfile.c_str());
    
    std::ofstream grfile;

    grfile.open(tbuf);
    grfile << "digraph {\n";
    int level = 0;

    GraphMoves(grfile,node,level);

    grfile << "}\n";
    grfile.close();

    // only small graphs can be processed by Graphviz dot program...

    sprintf(tbuf,"dot -Tpdf -o %s.pdf %s.dot",outfile.c_str(),outfile.c_str());

    // converting large graph takes long time and may not complete...
    //if (system(tbuf))
    //  std::cerr << "WARNING: Problem creating graph pdf?" << std::endl;

    std::cout << "\nTo create graph pdf use: '" << tbuf << "'" << std::endl;
#endif
 }
 
void MovesTree::GraphMoves(std::ofstream &grfile, MovesTreeNode *node, int level) {
#ifndef GRAPH_SUPPORT
    return;
#else  
    Board gb;

    std::stringstream this_vertex;
    std::string node_color_str;
    std::stringstream nlabel;

    node_color_str = (node->Color() == WHITE) ? "red" : "black";

    int node_id = node->ID();

    if (level == 0)
      this_vertex << "Root";
    else
      this_vertex << "N_" << node_id;

    nlabel /* << node->MoveScore() << "/" */ << node->Score();

    grfile << this_vertex.str() << "[color=\"" << node_color_str << "\",label=\"" << nlabel.str() << "\"," 
            << "fontcolor=\"" << node_color_str << "\"];\n"; 

    for (auto pmi = node->possible_moves.begin(); pmi != node->possible_moves.end(); pmi++) {
       MovesTreeNode *pm = *pmi;
       std::stringstream next_vertex;
       int pm_id = pm->ID();
       next_vertex << "N_" << pm_id;
       std::stringstream arc_label;
       Board game_board;
       arc_label << Engine::EncodeMove(game_board,*pm);
       std::string move_color_str = (pm->Color() == WHITE) ? "red" : "black";
       grfile << this_vertex.str() << " -> " << next_vertex.str() << "[label=\"" << arc_label.str() 
              << "\",color=\"" << move_color_str << "\"];\n";
       GraphMoves(grfile,&(*pm),level + 1);
    }
#endif  
 }

};
