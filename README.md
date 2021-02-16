Intro
------
Chess engine, written in C++, that may be used either stand-alone to play,
or may be used with gnu xboard.

Requirements
------------
The engine was developed using gnu C++, version 9.3.0, but any recent version
of C++ will do. The development platform was Ubuntu linux.

Usage
------
Use *cmake* to build engine executable.

To run the *engine* from xboard (assuming you have xboard installed):
```
   xboard -fcp ./my_engine -debug
```
The *-debug* option instructs xboard to create an xboard.debug file that includes a trace
of the game. Lines in the file prefixed by *usermove* are moves made by the user; lines
prefixed by *move* are responses from the chess engine.

To run the *engine* stand-alone*:
```
   >./my_engine        # start the engine, input will come from stdin
```
In *stand-alone* mode, the engine is waiting on input from stdin. The following commands
are recognized:
```
   showboard          # display the current game board
   
   usermove d2d4      # specify next move as xboard notation
   
   new                # use to start new game
   
   go                 # instructs engine to switch sides
   
   quit               # end execution
   
   changesides        # instruct engine to change sides
```
*The engine parses and supports just enough xboard commands to function. There are other
commands implemented that are required to use the engine with xboard. See stream_player.C.

There are some cmdline options. Use *./my_engine -h* to view options.

The engine can also be run from xboard, in *two machines* mode. Start xboard like so:

```
   xboard -fcp ./my_engine -scp ./my_engine
```

After xboard starts, use *Mode* pulldown, *Two Machines* option to start the game.

Status
-------
Engine is functional. Works with xboard, single and dual machine configs. Simple opening moves implemented.
Can recognize draw or checkmate. Can beat me, but I'm a lousy chess player (sigh).

Design
------
Engine uses Minimax algorithm, with alpha-beta tree pruning. During (minimax) tree traversal, sub-trees
are also deleted after processing, to conserve memory.

