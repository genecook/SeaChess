#include <string>
#include <stdexcept>
#include <unistd.h>
#include <sstream>
#include <iostream>
#include <stdlib.h>
#include <time.h>

#include "chess.h"

namespace SeaChess {

#ifndef NUMBER_OF_LEVELS
#define NUMBER_OF_LEVELS 3
#endif

//***********************************************************************************************
// chess Engine methods...
//***********************************************************************************************

void Engine::Init(int _num_levels,std::string _debug_enable_str, std::string _opening_moves_str,
		  std::string _load_file, unsigned int _move_time, std::string _algorithm) {
    color = BLACK;
    num_moves = 0;
    num_turns = 0;
    engine_debug = false;

    number_of_levels = (_num_levels > 0) ? _num_levels : NUMBER_OF_LEVELS;

    debug_move_trigger = _debug_enable_str;
    opening_moves_str = _opening_moves_str;

    have_opening_moves = false;

    NewGame();

    if (_load_file.size() > 0)
      Load(_load_file);

    srand(time(NULL));  

    if (_algorithm.size() == 0)
      algorithm_index = MINIMAX;      
    else if (_algorithm.find("mini") != std::string::npos)
      algorithm_index = MINIMAX;
    else if (_algorithm.find("rand") != std::string::npos)
      algorithm_index = RANDOM;
    else if (_algorithm.find("monte") != std::string::npos)
      algorithm_index = MONTE_CARLO;
    else {
      std::stringstream errmsg;
      errmsg << "#  Moves selection algorithm specified ('" << _algorithm << "') not recognized. ";
      throw std::logic_error(errmsg.str());
    }

    switch(algorithm_index) {
      case MINIMAX:     /* supported */ break;
      case MONTE_CARLO: /* supported */ break;
      case RANDOM:      /* supported */ break;
      default: { std::stringstream errmsg;
                 errmsg << "#  Moves selection algorithm specified not yet supported. ";
                 throw std::logic_error(errmsg.str());
               }
               break;
    }
    
    move_time = _move_time;
  };

//***********************************************************************************************
// choose next move...
//***********************************************************************************************

std::string Engine::ChooseMove(Board &game_board, Move *suggested_move) {
  //std::cout << "[ChooseMove] entered..." << std::endl;

  MovesTree *moves_tree;

  switch(Algorithm()) {
    case MINIMAX:     moves_tree = new MovesTreeMinimax(Color(), Levels());
                      break;
    case MONTE_CARLO: moves_tree = new MovesTreeMonteCarlo(Color(), Levels(), MoveTime());
                      break;
    case RANDOM:      moves_tree = new MovesTreeRandom(Color(), NumberOfTurns());
                      break;
    default: break;
  }

  Move next_move;
  int num_moves = moves_tree->ChooseMove(&next_move,game_board,suggested_move);
  
  std::cout << "#  number of moves evaluated: " << num_moves
            << ", approx memory usage in bytes: " << (sizeof(Move) * num_moves) << std::endl;
  
  std::string move_str = NextMoveAsString(&next_move);
  //std::cout << "[ChooseMove] exited, next move: '" << move_str << "'" << std::endl;

  delete moves_tree;
  
  return move_str;
}
  
//***********************************************************************************************
// user can specify opening move(s) at startup...
//***********************************************************************************************

void Engine::UserSetsOpening() {
  // always clear out the opening moves queue...
  while (!opening_moves.empty()) {
    opening_moves.pop();
  }
  // has user specified open moves?...
  if ( (opening_moves_str.size() == 0) || (opening_moves_str == " ") ) {
    // nope...
  } else {
    // yes...
    std::istringstream f(opening_moves_str);
    std::string oms;
    while(std::getline(f,oms,':')) {
      opening_moves.push(oms);
    }
    have_opening_moves = true;
  }
}

//***********************************************************************************************
// choose standard opening move...
//***********************************************************************************************

void Engine::ChooseOpening(std::string opponents_opening_move) {
  std::cout << "#  choose opening..." << std::endl;
  if (have_opening_moves) return; // already have opening...

  std::cout << "#  setup opening move..." << std::endl;

  if (opponents_opening_move == "?") {
    // we move first...
    int opick = rand() % 3;
    switch(opick) {
      case 0: opening_moves.push("e2e4"); 
              opening_moves.push("g1f3"); 
              opening_moves.push("f1e2"); 
              opening_moves.push("e1g1"); 
              opening_moves.push("d2d3"); 
              break;
      case 1: opening_moves.push("d2d4"); 
              opening_moves.push("e2e3");
              opening_moves.push("g1f3");
              opening_moves.push("f1b5");
              opening_moves.push("b5c6");
              opening_moves.push("e1g1");
              break;
      case 2: opening_moves.push("b1c3"); 
              opening_moves.push("d2d3");
              opening_moves.push("g1f3");
              opening_moves.push("e2e4");
              opening_moves.push("f1e2");
              opening_moves.push("e1g1");
              break;
      default: break;
    }
    have_opening_moves = true;
    return;
  }
  
  // opponent has made the first move...
  if (opponents_opening_move == "e2e4") {
    int opick = 0; //rand() % 3;
    switch(opick) {
      case 0: opening_moves.push("e7e5"); 
              opening_moves.push("d7d6");
              opening_moves.push("g8f6");
              opening_moves.push("f8e7");
              opening_moves.push("e8g8");
              break;

      case 1: opening_moves.push("c6c4"); 
      break;

      case 2: opening_moves.push("g8f6"); 
      break;
      default: break;
    }
  } else if (opponents_opening_move == "d2d4") {
    int opick = rand() % 3;
    switch(opick) {
      case 0: opening_moves.push("e7e6"); break;
      case 1: opening_moves.push("d7d5"); break;
      case 2: opening_moves.push("g8f6"); break;
      case 3: opening_moves.push("b8c6"); break;
      default: break;
    }
  } else if (opponents_opening_move == "b1c3") {
    opening_moves.push("g8f6");
  }

  have_opening_moves = true;
}

//***********************************************************************************************
//***********************************************************************************************

std::string Engine::NextOpeningMove() {
  std::string move_str;
  
  if (opening_moves.empty()) {
    // there are no opening moves to be made...
  } else {  
    // retreive next opening move...
    move_str = opening_moves.front();
    opening_moves.pop();
  }
  
  return move_str;
}

//***********************************************************************************************
// decode chess move in algebraic notation...
//***********************************************************************************************

void Engine::CrackMoveStr(int &start_row,int &start_column,int &end_row,int &end_column,std::string &move_str) {
  if (move_str.size() == 5) {
    if (move_str.substr(4,1) == "q") {
      // will ASSUME pawn promotion. 'q' indicates promotion to queen...
    } else {
      throw std::runtime_error("unsupported xboard move string???");
    }
  } else if (move_str.size() != 4)
    throw std::runtime_error("xboard move string is NOT four chars???");
  
  std::string src_coordinate_str = move_str.substr(0,2);
  std::string dest_coordinate_str = move_str.substr(2,2);
  
  game_board.Index(start_row,start_column,src_coordinate_str);
  game_board.Index(end_row,end_column,dest_coordinate_str);
}

//***********************************************************************************************
// cmdline option used to specify when to enable debug...
//***********************************************************************************************

void Engine::DebugEnable(std::string move_str) {
  if (move_str == "!") {
    std::cout << "# debug enabled now..." << std::endl;
    //engine_debug = true;
    return;
  }
    
  unsigned int which_turn = 0;
  
  if ( (sscanf(move_str.c_str(),"%u",&which_turn) == 1) && (num_turns >= which_turn) ) {
    std::cout << " debug enabled after " << which_turn << " moves..." << std::endl;
    //engine_debug = true;
    return;
  }
  
  if (move_str == debug_move_trigger) {
    std::cout << "move " << move_str << " has caused debug to be enabled..." << std::endl;
    //engine_debug = true;
  }
}

//***********************************************************************************************
// apply the opponents next move...
//***********************************************************************************************

std::string Engine::UserMove(std::string opponents_move_str) {
  int start_row = -1,start_column = -1,end_row = -1,end_column = -1; // move made prior to
  
  ChooseOpening(opponents_move_str);

  DebugEnable(opponents_move_str); // opponents move could enable debug
  
  // update board with opponents move, evaluate for possible checkmate...
  
  CrackMoveStr(start_row,start_column,end_row,end_column,opponents_move_str);

  Move omove(start_row,start_column,end_row,end_column,OpponentsColor());
  Board tmp_board = game_board; // in case of invalid move on users part
    
  try {
     tmp_board.MakeMove(start_row,start_column,end_row,end_column);
  } catch( std::logic_error reason) {
     std::cout << "# Invalid move, reason: '" << reason.what() << ". move ignored." << std::endl;
     return "Illegal move: " + opponents_move_str;
  }

  MovesTree moves_tree(Color(), Levels());

  if ( moves_tree.Check(tmp_board,OpponentsColor()) ) {
    std::cout << "# Invalid move for " << ColorAsStr(OpponentsColor())
	      << ". You are, or would be in check." << std::endl;
    return "Illegal move (in or moving into check): " + opponents_move_str;
  }

  std::vector<Move> all_possible_moves;
  
  moves_tree.GetMoves(&all_possible_moves,game_board,OpponentsColor());

  bool this_move_is_possible = false;
  
  for (auto pmi = all_possible_moves.begin(); pmi != all_possible_moves.end(); pmi++) {
    if (*pmi == omove) {
      this_move_is_possible = true;
      break;
    }
  }

  if (!this_move_is_possible) {
    std::cout << "# Invalid move for " << ColorAsStr(OpponentsColor()) << std::endl;
    return "Illegal move: " + opponents_move_str;
  }
  
  game_board = tmp_board;
  
  return "";
}

//***********************************************************************************************
// represent move outcome as string. side effect is to potentially enable debug...
//***********************************************************************************************

std::string Engine::NextMoveAsString(Move *next_move) {
  std::string next_move_str;

  std::cout << "#  Outcome: " << OutcomeAsStr(next_move->Outcome()) << std::endl;
 
  if (next_move->Outcome() == RESIGN) {
    std::cout << "#  " << ColorAsStr(Color()) << " resigns!" << std::endl;
    next_move_str = "resign";
  } else if (next_move->Outcome() == DRAW) {
    std::cout << "#  " << ColorAsStr(Color()) << " draw!" << std::endl;
    next_move_str = "1/2-1/2 {Draw by repetition}";
  } else if (next_move->Outcome() == CHECKMATE) {
    std::cout << "#  " << ColorAsStr(OpponentsColor()) << " is checkmated!" << std::endl;
    std::cout << "#  move that will cause mate: " + EncodeMove(game_board,next_move) << "\n" << std::endl;
    next_move_str = (OpponentsColor() == WHITE) ? "1-0 {Black mates}" : "0-1 {White mates}";
  } else {
    game_board.MakeMove(next_move->StartRow(),next_move->StartColumn(), // the root node 
  		        next_move->EndRow(),next_move->EndColumn());    //  contains the next move...
    next_move_str = "move " + EncodeMove(game_board,next_move);
    DebugEnable(next_move_str); // machine move could enable debug
  }

  return next_move_str;
}

//***********************************************************************************************
// make the next move...
//***********************************************************************************************

std::string Engine::NextMove() {
  ChooseOpening("?"); // opening may have already been chosen, but if not...
  
  std::string opening_move_str = NextOpeningMove();

  Move opening_move;
  
  if (opening_move_str.size() > 0) {
    std::cout << "#  next opening move: '" << opening_move_str << "'" << std::endl;
    int om_start_row,om_start_column,om_end_row,om_end_column;
    CrackMoveStr(om_start_row,om_start_column,om_end_row,om_end_column,opening_move_str);
    Move omove(om_start_row,om_start_column,om_end_row,om_end_column,Color());
    opening_move.Set(&omove);
    opening_move.SetOutcome(SIMPLE_MOVE);
  }

  num_turns++;

  return ChooseMove(game_board,&opening_move);
}

//***********************************************************************************************
// save or load board state from file...
//***********************************************************************************************

void Engine::Save(std::string saveFile) {
  std::ofstream oFile(saveFile,std::ios::out | std::ios::binary);
  
  oFile.write( (char *) &color, sizeof(unsigned char) );

  game_board.Save(oFile);

  oFile.write( (char *) &number_of_levels, sizeof(unsigned int) );
  oFile.write( (char *) &num_moves, sizeof(unsigned int) );
  oFile.write( (char *) &num_turns, sizeof(unsigned int) );
  oFile.write( (char *) &engine_debug, sizeof(bool) );
  oFile.write( (char *) debug_move_trigger.c_str(), debug_move_trigger.size() );

  oFile.close();
}

void Engine::Load(std::string loadFile) {
  std::ifstream iFile(loadFile,std::ios::in | std::ios::binary);

  iFile.read( (char *) &color, sizeof(unsigned char) );

  game_board.Load(iFile);

  iFile.read( (char *) &number_of_levels, sizeof(unsigned int) );
  iFile.read( (char *) &num_moves, sizeof(unsigned int) );
  iFile.read( (char *) &num_turns, sizeof(unsigned int) );
  iFile.read( (char *) &engine_debug, sizeof(bool) );
  iFile.read( (char *) debug_move_trigger.c_str(), debug_move_trigger.size() );

  iFile.close();
}

}
