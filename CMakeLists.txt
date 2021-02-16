cmake_minimum_required(VERSION 3.13)

project(SeaChess VERSION 0.1)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)

set(CMAKE_CXX_FLAGS "-std=c++11 -O3 -pthread")

include_directories(include)

add_executable(sea_chess src/main.C src/stream_player.C src/parse_cmdline_options.C)

add_library(sea_chess_lib src/board.C src/pieces.C src/bishop.C src/king.C src/knight.C
  src/pawn.C src/queen.C src/rook.C src/engine.C src/moves_tree.C src/eval_move.C
  src/move.C)

target_link_libraries(sea_chess sea_chess_lib)

install(TARGETS sea_chess_lib DESTINATION ${CMAKE_SOURCE_DIR}/lib)
install(TARGETS sea_chess DESTINATION ${CMAKE_SOURCE_DIR}/bin)

enable_testing()

add_test(NAME test1
         COMMAND sh -c "cat ${CMAKE_SOURCE_DIR}/tests/xboard.debug.unit_test | ${CMAKE_SOURCE_DIR}/utils/filter.xboard.debug.pl | ./sea_chess")

add_test(NAME test2
         COMMAND sh -c "cat ${CMAKE_SOURCE_DIR}/tests/xboard.debug.unit_test2 | ./sea_chess")

add_test(NAME test3
         COMMAND sh -c "cat ${CMAKE_SOURCE_DIR}/tests/xboard.debug.unit_test3 | ${CMAKE_SOURCE_DIR}/utils/filter.xboard.debug.pl | ./sea_chess")

add_test(NAME test4
         COMMAND sh -c "cat ${CMAKE_SOURCE_DIR}/tests/xboard.debug.checkmate2 | ${CMAKE_SOURCE_DIR}/utils/filter.xboard.debug.pl | ./sea_chess")

add_test(NAME test5
         COMMAND sh -c "cat ${CMAKE_SOURCE_DIR}/tests/xboard.debug.unit_test4 | ${CMAKE_SOURCE_DIR}/utils/filter.xboard.debug.pl | ./sea_chess")

add_test(NAME test6
         COMMAND sh -c "cat ${CMAKE_SOURCE_DIR}/tests/xboard.debug.unit_test | ${CMAKE_SOURCE_DIR}/utils/filter.xboard.debug.pl | ./sea_chess -n 5")

add_test(NAME test7
         COMMAND sh -c "cat ${CMAKE_SOURCE_DIR}/tests/xboard.debug.unit_test2 | ./sea_chess -n 5")

add_test(NAME test8
         COMMAND sh -c "cat ${CMAKE_SOURCE_DIR}/tests/xboard.debug.unit_test3 | ${CMAKE_SOURCE_DIR}/utils/filter.xboard.debug.pl | ./sea_chess -n 5")

add_test(NAME test9
         COMMAND sh -c "cat ${CMAKE_SOURCE_DIR}/tests/xboard.debug.checkmate2 | ${CMAKE_SOURCE_DIR}/utils/filter.xboard.debug.pl | ./sea_chess -n 5")

add_test(NAME test10
         COMMAND sh -c "cat ${CMAKE_SOURCE_DIR}/tests/xboard.debug.unit_test4 | ${CMAKE_SOURCE_DIR}/utils/filter.xboard.debug.pl | ./sea_chess -n 5")



