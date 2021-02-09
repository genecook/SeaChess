#include <iostream>
#include <chess.h>

namespace SeaChess {
  
// stream out Move object...

std::ostream& operator<< (std::ostream &os, Move &fld) {
  os << Board::Coordinates(fld.start_row,fld.start_column)
     << Board::Coordinates(fld.end_row,fld.end_column);
  os << " color: " << ColorAsStr(fld.color);
  os << " outcome: " << OutcomeAsStr(fld.outcome);
  os << " score: " << fld.score;
  os << " capture-type: " << CaptureTypeAsStr(fld.capture_type);
  return os;
}

/*
    os << " # of possible-moves: " << ( (fld.possible_moves != NULL) ? fld.possible_moves->size() : 0) ;
    os << "\n";
    int ix = 0;
    // stream out one level only...
    if (fld.possible_moves != NULL) {
      for (auto vi = fld.PossibleMovesBegin(); vi != fld.PossibleMovesEnd(); vi++) {
         Move *i = (Move *) *vi;
         os << "   next possible move[" << ix++ << "]:";
         os << " start row/column: " << (*i).start_row << "/" << (*i).start_column;
         os << " end row/column: " << (*i).end_row << "/" << (*i).end_column;
         os << " color: " << ChessUtils::ColorAsStr((*i).color);
         os << " outcome: " << ChessUtils::OutcomeAsStr(fld.outcome);
         os << "\n";
      }
    }
  } else {
    os << "Invalid!";
  }
  return os;
}

*/
}
