#ifndef __CHESS__

namespace SeaChess {
  enum PIECE_TYPE { NONE=0, PAWN, ROOK, KNIGHT, BISHOP, KING, QUEEN };

  enum COLOR { NOT_SET=0, WHITE, BLACK };

  enum INDICES { INVALID_INDEX=8 }; // board row/column indices range from 0..7
  
  enum OUTCOME { UNKNOWN=0, SIMPLE_MOVE, CAPTURE, PROMOTION, THREAT, CHECK, CHECKMATE, DRAW, RESIGN, SQUARE_BLOCKED };
};

#include <assert.h>
#include <chess_utils.h>
#include <board.h>
#include <move.h>
#include <pieces.h>
#include <moves_tree.h>
#include <engine.h>

#endif

#define __CHESS__
