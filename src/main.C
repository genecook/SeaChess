#include <iostream>
#include <string>

#include <chess.h>
#include <program_options.h>

namespace StreamPlayer {
  int Play(SeaChess::Engine *the_engine);
}

int main(int argc, char **argv) {
  std::cout << "#  my_engine, version: alpha\n";
  
  ProgramOptions my_options;

  if (!my_options.parse_cmdline_options(argc,argv)) {
    my_options.display_cmdline_options();
    exit(-1);
  }

  int engine_exit_code = 0;
  
  try {
    SeaChess::Engine my_little_engine(my_options.num_levels, my_options.debug_enable_str,
  				      my_options.opening_moves_str, my_options.load_file, 
				      my_options.move_time,my_options.algorithm);

    if (my_options.is_white) {
      std::cout << "# engine starts as white..." << std::endl;
      my_little_engine.ChangeSides();
    }
  
    engine_exit_code = StreamPlayer::Play(&my_little_engine);
  } catch( std::logic_error reason) {
    std::cout << reason.what() << std::endl;
    std::cout << "#  Program halted." << std::endl;
    exit(-1);
  }

  return engine_exit_code;
}
