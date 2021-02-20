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
// common moves tree methods...
//***********************************************************************************************

// stream out MovesTreeNode object...

std::ostream& operator<< (std::ostream &os, MovesTreeNode &fld) {
  os << Board::Coordinates(fld.start_row,fld.start_column)
     << Board::Coordinates(fld.end_row,fld.end_column);
  os << " color: " << ColorAsStr(fld.color);
  os << " outcome: " << OutcomeAsStr(fld.outcome);
  os << " capture-type: " << CaptureTypeAsStr(fld.capture_type);
  os << " # of visits: " << fld.NumberOfVisits();
  os << " # of white wins: " << fld.NumberOfWhiteWins();
  os << " # of black wins: " << fld.NumberOfBlackWins();
  return os;
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
    for (auto i = 0; i < node->PossibleMovesCount(); i++) {
       MovesTreeNode *pm = node->PossibleMove(i);
       Board updated_board = MakeMove(game_board,pm); 
       EvalBoard(pm,updated_board); // evaluate every move to yield raw score
    }
    node->Sort(movesortfunction);
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

bool bestmovesortfunction(MovesTreeNode *m1, MovesTreeNode *m2) {
  return (m1->Score() > m2->Score());
}
  
void MovesTree::PickBestMove(MovesTreeNode *root_node, Board &game_board, Move *suggested_move) {
  if (root_node->PossibleMovesCount() == 0) {
    // this is the root node. no moves can be made. will ASSUME draw or checkmate..."
    root_node->SetOutcome(RESIGN);
  }
  
  // sort from best move to worst, according to score...
  
  root_node->Sort( bestmovesortfunction );

  // assume 1st score (after sort) will be the best...
  
  MovesTreeNode *best_move = root_node->PossibleMove(0);
  root_node->Set( best_move );
  
  std::cout << "BEST MOVE: " << (*best_move) << std::endl;
  
  // is there a suggested move?...

  if (suggested_move == NULL)
    return; // nope.

  // yes. see if this move corresponds to one of the move choices...
  
  for (auto i = 0; i < root_node->PossibleMovesCount(); i++) {
     MovesTreeNode *pm = root_node->PossibleMove(i);
     // if the suggested move is located and its score is as least as good as the best score
     // OR the best move isn't too interesting, then use the suggested move...
     if ( pm->Match(suggested_move) ) {
       std::cout << "SUGGESTED MOVE SCORE: " << pm->Score() << std::endl;
       if ( (pm->Score() == best_move->Score()) || (best_move->Outcome() == SIMPLE_MOVE) ) {
	 // it does. lets use the suggested move, and hope its a good one...
         root_node->Set(pm);
	 break;
       }
     }
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

    nlabel << node->NumberOfVisits() << "/" << node->NumberOfWins(node->Color());

    grfile << this_vertex.str() << "[color=\"" << node_color_str << "\",label=\"" << nlabel.str() << "\"," 
            << "fontcolor=\"" << node_color_str << "\"];\n"; 

    for (auto pmi = 0; pmi < node->PossibleMovesCount(); pmi++) {
       MovesTreeNode *pm = node->PossibleMove(pmi);
       std::stringstream next_vertex;
       next_vertex << "N_" << pm->ID();
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
