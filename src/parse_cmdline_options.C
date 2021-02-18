#include <iostream>
#include <string>
#include <string.h>

#include <program_options.h>

//********************************************************************************
const char *help_text = "\n\
  my_engine  - simple(minded) chess engine, alpha version.\n\n\
\
    cmdline args:\n\
      -d <start>      -- enable moves-tree debug, where <start> is # of turns or some specific move to start on.\n\
      -o <opening>    -- supply colon-separated list of opening moves for machine.\n\
      -L <file>       -- load game state from file.\n\
      -W              -- start as white (defaults to black).\n\
      -n <levels>     -- number of move evaluation levels. (default is four)\n\
      -A              -- algorithm to use (default is minimax)\n\
      -t <seconds>    -- time alloted to each (computer) move, in seconds (monte-carlo only)\n\
\n\
    examples:\n\
      my_engine -n 5           -- specify five levels of moves evaluation, for every machine move to be made.\n\
\n\
      my_engine -d b4d6        -- enable moves-tree debug when this move seen\n\
\n\
      my_engine -d 10          -- enable moves-tree debug after 10 moves made\n\
\n\
      my_engine -o 'e2e4:g1f3' -- specify 1st two moves for machine\n\
\n\
      my_engine -o ''          -- disable machine default opening moves\n\
\n\
      my_engine -A random      -- moves selected randomly\n\
\n\
      my_engine -A monte-carlo -- use monte-carlo tree simulation to select moves\n\
\n\
      my_engine -t 20          -- limit time to select moves to 20 seconds (monte-carlo only)\n\
";
//********************************************************************************

void ProgramOptions::display_cmdline_options() {
    std::cout << help_text << std::endl;
}

bool ProgramOptions::parse_cmdline_options(int argc, char **argv) {
  bool options_okay = true;

  for (int i = 1; (i < argc) && options_okay; i++) {
    if (!strcmp(argv[i],"-h")) {
      options_okay = false;
      continue;
    }
    
    if (!strcmp(argv[i],"-n")) {
      int _num_levels;
      if ( ++i >= argc) {
	std::cout << "'-n' cmdline arg specified without level." << std::endl;
	options_okay = false;
      } else if (sscanf(argv[i],"%u",&_num_levels) < 1) {
	std::cout << "Invalid level value specified with '-n' cmdline arg." << std::endl;
	options_okay = false;
      } else {
	num_levels = _num_levels;
	std::cout << "    # levels: " << num_levels << std::endl;
      }
      continue;
    }
    
    if (!strcmp(argv[i],"-d")) {
      if ( ++i >= argc) {
	    std::cout << "'-d' cmdline arg specified without start arg." << std::endl;
	    options_okay = false;
      } else {
        debug_enable_str = argv[i];
	    std::cout << "    # debug-trigger: " << debug_enable_str << std::endl;
      }
      continue;
    }
    
    if (!strcmp(argv[i],"-o")) {
      if ( ++i >= argc) {
	    std::cout << "'-o' cmdline arg specified without opening moves string" << std::endl;
	    options_okay = false;
      } else {
        opening_moves_str = argv[i];
	      if ( (opening_moves_str.size() == 0) || (opening_moves_str == " ") ) {
	        std::cout << "    # opening moves disabled." << std::endl;
	        opening_moves_str = " "; // single blank conveys to engine to flush any default opening moves
	      } else {
	        std::cout << "    # opening moves: " << (opening_moves_str==" " ? "<none>" : opening_moves_str) << std::endl;
	      }
      }
      continue;
    } 
    
    if (!strcmp(argv[i],"-L")) {
      if ( ++i >= argc) {
	    std::cout << "'-L' cmdline arg specified without filename." << std::endl;
	    options_okay = false;
      } else {
        load_file = argv[i];
	    std::cout << "    # file to load game state from: " << load_file << std::endl;
      }
      continue;
    }
    
    if (!strcmp(argv[i],"-A")) {
      if ( ++i >= argc) {
	    std::cout << "'-A' cmdline arg specified without algorithm name." << std::endl;
	    options_okay = false;
      } else {
	algorithm = argv[i];
	std::cout << "#  algorithm specified: " << algorithm << std::endl;
      }
      continue;
    }
    
    if (!strcmp(argv[i],"-t")) {
      if ( ++i >= argc) {
	      std::cout << "'-t' cmdline arg specified without count." << std::endl;
	      options_okay = false;
      } else if (sscanf(argv[i],"%u",&move_time) < 1) {
	      std::cout << "Invalid value specified with '-t' cmdline arg." << std::endl;
	      options_okay = false;
      } else {
	      std::cout << "    # move time in seconds: " << move_time << std::endl;
      }
      continue;
    }
    
    if (!strcmp(argv[i],"-W")) {
      is_white = true;
      continue;
    }

    std::cout << "Unrecognized cmdline option: " << std::string(argv[i]) << std::endl;
    options_okay = false;
  }

  if (options_okay)
    std::cout << std::endl;
  
  return options_okay;
}
