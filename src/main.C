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
    
  SeaChess::Engine my_little_engine(my_options.num_levels, my_options.debug_enable_str,
				    my_options.opening_moves_str, my_options.load_file, 
				    my_options.move_time);

  if (my_options.is_white) {
    std::cout << "# engine starts as white..." << std::endl;
    my_little_engine.ChangeSides();
  }
  
  return StreamPlayer::Play(&my_little_engine);
}
