#ifndef __MOVE__

//******************************************************************************
// Move - represents a single chess move
//******************************************************************************

#include <iostream>
#include <vector>

namespace SeaChess {

class Move {
public:
  Move() { InitMove(); };

  Move(int _start_row, int _start_column, int _end_row, int _end_column, int _color,
       int _outcome = INVALID_INDEX, int _capture_type = INVALID_INDEX) {
    InitMove(_start_row,_start_column,_end_row,_end_column,_color,_outcome,false,_capture_type); 
  };

  ~Move() {};

  void InitMove(int _start_row = INVALID_INDEX, int _start_column = INVALID_INDEX, int _end_row = INVALID_INDEX,
	        int _end_column = INVALID_INDEX, int _color = NOT_SET, int _outcome = INVALID_INDEX, int _check = false,
	        int _capture_type = UNKNOWN) {
    start_row = _start_row;
    start_column = _start_column;
    end_row = _end_row;
    end_column = _end_column;
    color = _color;
    outcome = _outcome;
    check = false;
    capture_type = _capture_type;
    score = 0;
  };

 void Set(const Move *src) {
    start_row          = src->start_row;
    start_column       = src->start_column;
    end_row            = src->end_row;
    end_column         = src->end_column;
    color              = src->color;
    outcome            = src->outcome;
    check              = src->check;
    capture_type       = src->capture_type;
    score              = src->score;
  };

  // move comparison...

  bool operator==( const Move &rhs ) {
    return (start_row == rhs.start_row) && (start_column == rhs.start_column)
            && (end_row == rhs.end_row) && (end_column == rhs.end_column)
            && (color == rhs.color);
  };

  // move copy constructor...

/* copy by value is entirely reasonable at this point in design...
  Move(const Move &src) {
    start_row    = src.start_row;
    start_column = src.start_column;
    end_row      = src.end_row;
    end_column   = src.end_column;
    color        = src.color;
    outcome      = src.outcome;
    capture_type = src.capture_type;
    check        = src.check;
  };
*/
  
  int  StartRow()    { return start_row;    }; 
  int  StartColumn() { return start_column; };
  int  EndRow()      { return end_row;      };
  int  EndColumn()   { return end_column;   };
  int  Color()       { return color;        };
  int  Outcome()     { return outcome;      };
  bool Check()       { return check;        };
  
  void SetColor(int _color) { color = _color; };
  void SetOutcome(int _outcome) { outcome = _outcome; };
  void SetCheck(bool _check=true) { check = _check; };

  int Score() { return score; };
  void SetScore(int _score) {
    if ( (_score < INT_LEAST16_MIN) || (_score > INT_LEAST16_MAX) ) {
      std::cout << INT_LEAST16_MIN << "/" << _score << "/" << INT_LEAST16_MAX << std::endl;
    }
    assert ( (_score >= INT_LEAST16_MIN) && (_score <= INT_LEAST16_MAX) );
    score = _score;
  };

  void SetInvalid() { start_row = INVALID_INDEX; };
  
  bool Valid() { return (start_row < INVALID_INDEX) && (start_column < INVALID_INDEX); };

  int CaptureType() { return capture_type; };
  void SetCaptureType(int type) { capture_type = type; };

  // match on src/dest position, color...
  bool Match(Move *src) {
    return (color == src->color) && (start_row == src->start_row) && (start_column == src->start_column)
          && (end_row == src->end_row) && (end_column == src->end_column);
  };

  // tbd: chess pieces can set/query 'castling blocked' state...
  bool CastlingBlocked(int _color) { return castle_block_color == _color; };
  void SetCastlingBlocked(int _color_blocked) { castle_block_color = _color_blocked; };

  friend std::ostream& operator<< (std::ostream &os, SeaChess::Move &fld);

protected:
  unsigned int  start_row          : 4; // the move made - board start
  unsigned int  start_column       : 4; //
  unsigned int  end_row            : 4; //        "          "   end
  unsigned int  end_column         : 4; //
  unsigned int  color              : 2; // black or white

  unsigned int  outcome            : 4; // impact of move - just a move? piece capture? check? checkmate?
  unsigned int  capture_type       : 4; // type of piece captured or threatened

  unsigned int  check              : 1; // set during 
  unsigned int  castle_block_color : 2; //    possible-moves generation; check at eval time.

  int_least16_t score;                  // score - ranges from INT_LEAST16_MIN to INT_LEAST16_MAX
};

};
  
#endif
#define __MOVE__
