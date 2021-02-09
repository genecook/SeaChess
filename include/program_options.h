#ifndef __PROGRAM_OPTIONS__
#include <string>

// cmdline options...

struct ProgramOptions {
    ProgramOptions() : num_levels(0), max_levels(3), is_white(false),move_time(20) {};

    bool parse_cmdline_options(int argc, char **argv);

    void display_cmdline_options();
    
    int num_levels;
    int max_levels;
    std::string debug_enable_str;
    std::string opening_moves_str;
    std::string load_file;
    bool is_white;
    unsigned int move_time;  // time allowed to make a move, in seconds
};

#endif
#define __PROGRAM_OPTIONS__

