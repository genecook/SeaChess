#ifndef __ENGINE__

#include <string>
#include <vector>
#include <queue>
#include <iostream>
#include <fstream>
#include <sys/time.h>
#include <math.h>

namespace SeaChess {
  
//***************************************************************************************
// chess Engine class def...
//***************************************************************************************

enum ALGORITHMS { MINIMAX=0, RANDOM, MONTE_CARLO };

class Engine {
 public:
  Engine() : algorithm_index(MINIMAX) {};
  Engine(int _num_levels,std::string _debug_enable_str, std::string _opening_moves_str,
	 std::string _load_file, unsigned int _move_time, std::string _algorithm) : algorithm_index(MINIMAX) {
    Init(_num_levels,_debug_enable_str,_opening_moves_str,_load_file, _move_time, _algorithm);
  };
  ~Engine() {};

  void Init(int _num_levels,std::string _debug_enable_str, std::string _opening_moves_str,
	    std::string _load_file, unsigned int _move_time, std::string algorithm);
  
  // these public methods represent the engine 'api':
  
  virtual void NewGame() {
    color = BLACK;
    game_board.Setup();
    num_moves = 0;
    num_turns = 0;
    UserSetsOpening();
  };

  std::string NextMove();

  int Algorithm() { return algorithm_index; };
  
  std::string ChooseMove(Board &game_board, Move *suggested_move = NULL);

  void ChangeSides() {
    color = (color==WHITE) ? BLACK : WHITE;
    UserSetsOpening();
  };

  void SetColor(int _color) {
    color = _color;
    UserSetsOpening();
  };

  static int OtherColor(int current_color) { return (current_color == WHITE) ? BLACK : WHITE; };

  // update board with opponents move...
  
  std::string UserMove(std::string opponents_move);

  // display the game board, current state...
  
  void ShowBoard() {
    std::cout << "# game board:\n" << game_board << std::endl;
  };

  void Save(std::string saveFile);
  void Load(std::string loadFile);

  void SetDebug(bool _debug) { engine_debug = _debug; };
  bool Debug() { return engine_debug; };
  
  // encode move in algebraic notation...
  
  static std::string EncodeMove(Board &game_board,Move &src) {
    return game_board.Coordinates(src.StartRow(),src.StartColumn())
      + game_board.Coordinates(src.EndRow(),src.EndColumn());
  };
  static std::string EncodeMove(Board &game_board,Move *src) {
    return game_board.Coordinates(src->StartRow(),src->StartColumn())
      + game_board.Coordinates(src->EndRow(),src->EndColumn());
  };

protected:
  
  // user can specify opening move(s) at startup...

  void UserSetsOpening();

  // choose standard opening moves...
  
  void ChooseOpening(std::string first_move);

  // retreive next opening move...
  std::string NextOpeningMove();

  std::string NextMoveAsString(Move *next_move);

  // print move details...
  
  void ShowMove(std::string title, Board &board, Move *pv) {
    int type, color;
    if ( !board.GetPiece(type,color,pv->StartRow(),pv->StartColumn()) )
      std::runtime_error("ShowMove: no piece at start location!");
  
    std::cout << title << "move " << ColorAsStr(color) << " "
	    << PieceName(type) << " from "
	    << board.Coordinates(pv->StartRow(),pv->StartColumn())
            << " (" << pv->StartRow() << "/" << pv->StartColumn() << ")"
	    << " to " << board.Coordinates(pv->EndRow(),pv->EndColumn())
	    << " (" << pv->EndRow() << "/" << pv->EndColumn() << ")";
    std::cout << std::endl;
  };

  // user has some control over debug prints...
  
  void DebugEnable(std::string move_str);

  int MoveTime() { return move_time; };
  
  // return color assigned to engine...
  
  int Color() { return color; };
  int OpponentsColor() { return (Color() == WHITE) ? BLACK : WHITE; };

  // the number of traversal levels can be controlled...

  void SetLevels(int _levels) { number_of_levels = _levels; };
  int Levels() { return number_of_levels; };
  int NextLevel() { number_of_levels++; return number_of_levels; };
  int PreviousLevel() { number_of_levels--; return number_of_levels; };

  // break down algebraic move into start/end board coordinates...
  
  void CrackMoveStr(int &start_row,int &start_column,int &end_row,int &end_column,
		    std::string &move_str);

  void StartClock() {
    gettimeofday(&t1,NULL);
  };

  double ElapsedTime() {
    struct timeval t2; 
    gettimeofday(&t2,NULL);
    return ((t2.tv_sec - t1.tv_sec) * 1000.0); // milliseconds part is close enough
  };

  bool Timeout(unsigned int move_time_in_seconds) {
    return ElapsedTime() > (double) move_time_in_seconds * 1000.0;
  };

  int NumberOfTurns() { return num_turns; };

 private:
  unsigned char color;                     // color assigned to engine
  Board         game_board;                // the game board
  unsigned int  number_of_levels;          // how many levels to look ahead
  unsigned int  num_moves;                 // # of moves examined for each play by the engine
  unsigned int  num_turns;                 // # of turns in a game (i move, then you move...)
  bool          engine_debug;              // debug flag
  std::string   debug_move_trigger;        // encoded move that could enable debug
  std::string   opening_moves_str;         // passed in string of opening moves

  int           algorithm_index;           // algorithm to use in choosing moves
  struct timeval t1;                       // used to time moves (monte-carlo)
  double elapsed_time;                     

  unsigned int move_time;                  // move time in seconds

  bool have_opening_moves;                 // set to true once opening moves have been set

  std::queue<std::string> opening_moves;   // 'machine side' opening moves
};

};

#endif
#define __ENGINE__
