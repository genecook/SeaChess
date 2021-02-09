#ifndef __MOVETREE__

//******************************************************************************
// MovesTree
//******************************************************************************

#include <iostream>
#include <vector>

namespace SeaChess {

class MovesTreeNode : public Move {
public:
  MovesTreeNode() { InitMove(); };
  
  MovesTreeNode(int start_row, int start_column, int end_row, int end_column, int color,
		int outcome = INVALID_INDEX, int capture_type = INVALID_INDEX) {
    InitMove(start_row, start_column, end_row, end_column, color, outcome, capture_type);
  };

  MovesTreeNode(Move move) {
    InitMove(move.StartRow(), move.StartColumn(), move.EndRow(), move.EndColumn(),
	     move.Color(), move.Outcome(), move.Check(), move.CaptureType());
  };
  
  ~MovesTreeNode() { Flush(); };

  MovesTreeNode *AddMove(Move new_move) {
    MovesTreeNode *new_node = new MovesTreeNode(new_move);
    possible_moves.push_back(new_node);
    return possible_moves.back();
  };
  
  void Flush() {
    for (auto pmi = possible_moves.begin(); pmi != possible_moves.end(); pmi++) {
      (*pmi)->Flush();
       delete *pmi;
    }
    possible_moves.erase(possible_moves.begin(),possible_moves.end());
  };

  std::vector<MovesTreeNode *> possible_moves;  
};

struct piece_counts {
    piece_counts() : kings(0), queens(0), bishops(0),knights(0),rooks(0),pawns(0) {};

    void dump(const std::string &prefix) {
      std::cout << prefix << "# kings/queens/bishops/knights/rooks/pawns: " << kings
      << "/" << queens << "/" << bishops << "/" << knights << "/" << rooks
      << "/" << pawns << std::endl;
    };

    int kings;
    int queens;
    int bishops;
    int knights;
    int rooks;
    int pawns;
};

class MovesTree {
 public:
  MovesTree(int _color, int _max_levels) : color(_color), max_levels(_max_levels) {
    root_node = new MovesTreeNode;
  };
  ~MovesTree() { root_node->Flush(); delete root_node; };

  int ChooseMove(Move *next_move, Board &game_board,Move *suggested_move);

  void ChooseMoveInner(MovesTreeNode *current_node, Board &current_board, int current_color, int current_level);

  void EvalMove(MovesTreeNode *move, Board &current_board, int forced_score=UNKNOWN);
  int  ScoreMove(MovesTreeNode *move, Board &current_board, int forced_score=UNKNOWN);

  bool GetMoves(std::vector<Move> *possible_moves, Board &game_board, int color,bool avoid_check = true);
  bool GetMoves(MovesTreeNode *current_node, Board &current_board, int current_color, bool avoid_check = true);

  void PickBestMove(MovesTreeNode *root_node, Board &game_board, Move *suggested_move);
  bool BestScore(MovesTreeNode *this_move, MovesTreeNode *previous_move);

  Board MakeMove(Board &board, MovesTreeNode *pv);
  bool Check(Board &board,int color);
  void CountPieces(struct piece_counts &counts, Board &game_board,int color);
  int GetPieceCount(Move *node,Board &game_board,int color);

  int Color() { return color; };
  int NextColor(int current_color) { return (current_color == WHITE) ? BLACK : WHITE; };
  
  int MaxLevels() { return max_levels; };
  
 protected:
  MovesTreeNode *root_node;

  int color;
  int max_levels;

  int eval_count;

  int kings_row;
  int kings_column;

  Pieces pieces; // used to generate moves for each chess piece type
};

};

#endif
#define __MOVETREE__
