cmake_minimum_required(VERSION 3.13)

project(SeaChess VERSION 0.1)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)

set(CMAKE_CXX_FLAGS "-std=c++11 -g -pthread")

include_directories(include)

add_executable(sea_chess src/main.C src/stream_player.C src/parse_cmdline_options.C)

add_library(sea_chess_lib src/board.C src/pieces.C src/bishop.C src/king.C src/knight.C
  src/pawn.C src/queen.C src/rook.C src/engine.C src/moves_tree.C src/eval_move.C
  src/move.C)

target_link_libraries(sea_chess sea_chess_lib)

install(TARGETS sea_chess_lib DESTINATION ${CMAKE_SOURCE_DIR}/lib)
install(TARGETS sea_chess DESTINATION ${CMAKE_SOURCE_DIR}/bin)
