#include <iostream>
#include "Perft.h"
#include <sstream>
#include "chessBoard.h"
#include "moveEngine.h"
#include <cctype>

using namespace std;

// global object for the game state
Chessboard game;

// global search object to run the engine
MoveEngine moveEngine;

Perft perftTest;

// global variables for window width and height
int width = 640;
int height = 480;

// global variables for board drawing parameters
double board_x = 0.5 * (width - height);
double board_y = 0;
double board_size = height;

double square_size = board_size * 0.9 * 0.125;
double square_x = board_x + board_size * 0.05;
double square_y = board_y + board_size * 0.05;

// global variables to keep track of the selected square
int select_x = -1;
int select_y = -1;

// global varibales to keep track of promotions
int promote_x = -1;
int promote_y = -1;

// global variabls to keep track of AI moves
bool undo = false;
uint8_t AI_turn = BLACK;
int AI_depth = 5;

// stack of previous moves
Move moveStack[1000];
int moveIndex = 0;

// function to implement the universal chess interface
void UCI()
{
	while (true)
	{
		string command = "";
		getline(cin, command);

		if (command == "uci")
		{
			cout << "id name Dragon Engine\n";
			cout << "id author KendragonX\n";
			cout << "option name UseNNUE type check default false\n";
			cout << "uciok\n";
		}
		if (command.substr(0, 9) == "setoption")
		{
			if (command.find("UseNNUE") != string::npos)
			{
				bool enabled = command.find("true") != string::npos;
				moveEngine.setUseNNUE(enabled);
				moveEngine.clearTT();
			}
		}
		if (command.substr(0, 7) == "isready")
		{
			cout << "readyok\n";
		}
		if (command.substr(0, 10) == "ucinewgame")
		{
			game.newGame();
			moveEngine.newSearchGame();
		}
		if (command.substr(0, 8) == "position")
		{
			// set up aa stringstream
			stringstream ss;
			ss.str(command.substr(9));

			// read the starting position
			string posString;
			string str = "";
			while (ss >> str)
			{
				if (str == "moves") break;
				posString += str + " ";
			}

			// set up the position
			if (posString.substr(0, 8) == "startpos")
			{
				// starting position is a new game
				game.newGame();
			}
			else if (posString.substr(0, 3) == "fen")
			{
				// starting position specified by fen string
				game.setPosition(posString.substr(4));
			}

			// make moves from the given position
			string moveString = "";
			while (ss >> moveString)
			{
				Move move;
				int x1 = moveString[0] - 'a';
				int y1 = 7 - moveString[1] + '1';
				int x2 = moveString[2] - 'a';
				int y2 = 7 - moveString[3] + '1';

				move.from = x1 + 8 * y1;
				move.to = x2 + 8 * y2;
				move.promotion = EMPTY;

				if (moveString.length() > 4)
				{
					char promo = static_cast<char>(tolower(static_cast<unsigned char>(moveString[4])));

					switch (promo)
					{
					case 'q':
						move.promotion = game.turn() == WHITE ? WHITE_QUEEN : BLACK_QUEEN;
						break;
					case 'r':
						move.promotion = game.turn() == WHITE ? WHITE_ROOK : BLACK_ROOK;
						break;
					case 'b':
						move.promotion = game.turn() == WHITE ? WHITE_BISHOP : BLACK_BISHOP;
						break;
					case 'n':
						move.promotion = game.turn() == WHITE ? WHITE_KNIGHT : BLACK_KNIGHT;
						break;
					default:
						break;
					}
				}
				game.move(move);
			}
		}
		if (command.substr(0, 2) == "go")
		{
			// set up a stringstream
			stringstream ss;
			ss.str(command.substr(3));

			Move bestMove;
			
			// get search depth
			int engineDepth = 0;
			int moveTime= 0;
			int wtime = 0;
			int btime = 0;
			int winc = 0;
			int binc = 0;

			string token = "";
			while (ss >> token)
			{
				if (token == "depth") ss >> engineDepth;
				else if (token == "movetime") ss >> moveTime;
				else if (token == "wtime") ss >> wtime;
				else if (token == "btime") ss >> btime;
				else if (token == "winc") ss >> winc;
				else if (token == "binc") ss >> binc; 
			}

			int timeForMove = 0;

			if(moveTime > 0)
			{
				timeForMove = moveTime;
			}
			else
			{
				int remaining = game.turn() == WHITE ? wtime : btime;
				int increment = game.turn() == WHITE ? winc : binc;

				if(remaining > 0){ timeForMove = remaining / 30 + increment /2; }
			}

			if(timeForMove <= 0)
			{
				engineDepth = engineDepth > 0 ? engineDepth : 4;
				bestMove = moveEngine.iterativeDeepening(game, engineDepth);
			}
			else
			{
				bestMove = moveEngine.iterativeDeepeningTimed(game, timeForMove);
			}

			// print the move to the console

			if (bestMove.from == 0 && bestMove.to == 0 && bestMove.promotion == EMPTY)
			{
				bestMove = moveEngine.firstLegalMove(game);
			}

			if (bestMove.from == 0 && bestMove.to == 0 && bestMove.promotion == EMPTY)
			{
				cout << "bestmove 0000\n";
			}
			else
			{
				string moveString = moveEngine.moveToString(bestMove);
				cout << "bestmove " << moveString << "\n";
			}
		}
			
		if (command.substr(0, 4) == "quit")
		{
			exit(0);
		}
		if(command.substr(0, 5) == "perft")
		{
			stringstream ss;
			ss.str(command.substr(6));

			int depth = 0;
			ss >> depth;

			uint64_t nodes = perftTest.perftDivide(game, depth);
			cout << "Nodes: " << nodes << endl;
		}
		if(command.substr(0, 8) == "clearall")
		{
			moveEngine.newSearchGame();
		}
		if (command.substr(0, 7) == "usennue")
		{
			moveEngine.setUseNNUE(true);
			moveEngine.clearTT();

			if (!moveEngine.isNNUELoaded())
			{
				if (moveEngine.loadNNUE("..\\..\\tiny_trained.dnnue"))
				{
					cout << "info string NNUE enabled and loaded default tiny_trained.dnnue\n";
				}
				else
				{
					cout << "info string NNUE enabled but default NNUE load failed\n";
				}
			}
		else
			{
				cout << "info string NNUE enabled and already loaded\n";
			}
		}
		if(command.substr(0, 12) == "useclassical")
		{
			moveEngine.setUseNNUE(false);
			moveEngine.clearTT();
			cout << "info string classical eval enabled\n";
		}
		if(command.substr(0, 4) == "eval")
		{
			int classicalWhite = moveEngine.staticEvalWhite(game);
			int classicalStm = moveEngine.staticEval(game);
			int nnueWhite = moveEngine.nnueEvalWhite(game);
			int nnueStm = moveEngine.nnueEval(game);
			int activeWhite = moveEngine.currentEvalWhite(game);
			int activeStm = moveEngine.currentEval(game);

			cout << "eval classical white " << classicalWhite
				<< " stm " << classicalStm
				<< " nnue white " << nnueWhite
				<< " stm " << nnueStm
				<< " active " << (moveEngine.isUsingNNUE() ? "nnue" : "classical")
				<< " white " << activeWhite
				<< " stm " << activeStm
				<< " nnueloaded " << (moveEngine.isNNUELoaded() ? true : false)
				<< endl;
		}
		if(command.substr(0, 9) == "traineval")
		{
			cout << "traineval " << moveEngine.staticEvalWhite(game) << endl;
		}		
		if(command.substr(0, 8) == "loadnnue")
		{
			stringstream ss;
			ss.str(command.substr(9));

			string path = "";
			ss>>path;

			if(moveEngine.loadNNUE(path))
			{
				cout<< "info string NNUE loaded " << path << endl;
			}
			else
			{
				cout << "info string NNUE load failed " << path  << endl;
			}
		}
		if(command.substr(0, 10) == "searcheval")
		{
			int depth = 3;

			if (command.size() > 11)
			{
				depth = stoi(command.substr(11));
			}

			SearchResult result = moveEngine.searchRoot(game, depth);

			cout << "searcheval " << result.score << endl;
		}

	}
}

// main function
int main(int argc, char** argv)
{
	cout << "type uci: ";
	string mode = "";
	getline(cin, mode);
	if (mode == "uci")
	{
		cout << "id name Dragon Engine\n";
		cout << "id author KendragonX\n";
		cout << "option name UseNNUE type check default false\n";
		cout << "uciok\n";

		moveEngine.initMoveEngine();

		UCI();
	}
	
	return 0;
}
