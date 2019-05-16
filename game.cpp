#include <iostream>
#include <vector>
#include <limits>

#include "agent.h"

#define BLACK_IS_AI true
#define WHITE_IS_AI true

// feature: thread, alpha-beta pruncing, timer, auto-play

using namespace std;

struct Movement {
	int x, y;
	char direction;
};

class Game {
public:
	Game() {
		pieces[0].push_back(Coordinate{ 0, 1 });
		pieces[0].push_back(Coordinate{ 0, 3 });
		pieces[0].push_back(Coordinate{ 0, 5 });
		pieces[0].push_back(Coordinate{ 0, 7 });
		pieces[0].push_back(Coordinate{ 1, 2 });
		pieces[0].push_back(Coordinate{ 1, 4 });
		pieces[0].push_back(Coordinate{ 1, 6 });
		pieces[0].push_back(Coordinate{ 2, 3 });
		pieces[0].push_back(Coordinate{ 2, 5 });
		pieces[1].push_back(Coordinate{ 5, 2 });
		pieces[1].push_back(Coordinate{ 5, 4 });
		pieces[1].push_back(Coordinate{ 6, 1 });
		pieces[1].push_back(Coordinate{ 6, 3 });
		pieces[1].push_back(Coordinate{ 6, 5 });
		pieces[1].push_back(Coordinate{ 7, 0 });
		pieces[1].push_back(Coordinate{ 7, 2 });
		pieces[1].push_back(Coordinate{ 7, 4 });
		pieces[1].push_back(Coordinate{ 7, 6 });
		moveCount = 0;
		whoseMove = 0;
	}
	void printMap() {
		int piecesIndex = 0;
		cout << endl << "  ";
		for (int j = 0; j < 17; j++) {
			cout << "-";
		}
		cout << endl;
		for (int i = 0; i < 8; i++) {
			cout << "  " << "|";
			for (int j = 0; j < 8; j++) {
				int printFlag = what_on_this(j, 7 - i);

				if (printFlag == 1) {
					if (whoseMove) {
						cout << "B";
					}
					else {
						for (int k = 0; k < pieces[whoseMove].size(); k++) {
							if (pieces[whoseMove][k].x == j && pieces[whoseMove][k].y == 7 - i) {
								cout << k;
								break;
							}
						}
					}
				}
				else if (printFlag == -1) {
					if (whoseMove) {
						for (int k = 0; k < pieces[whoseMove].size(); k++) {
							if (pieces[whoseMove][k].x == j && pieces[whoseMove][k].y == 7 - i) {
								cout << k;
								break;
							}
						}
					}
					else {
						cout << "W";
					}
				}
				else {
					for (int k = 0; ; k++) {
						if (k == movements.size()) {
							cout << " ";
							break;
						}
						else if (movements[k].x == j && movements[k].y == 7 - i) {
							cout << movements[k].direction;
							break;
						}
					}
				}
				cout << "|";
			}
			cout << endl;
		}
		cout << "  ";
		for (int j = 0; j < 17; j++) {
			cout << '-';
		}
		cout << endl << endl << endl;

	}
	bool isOver() {
		if (moveCount > 399 || pieces[0].size() == 0 || pieces[1].size() == 0) {
			return 1;
		}
		for (int i = 0; ; i++) {
			if (i == pieces[0].size()) {
				return 1;
			}
			else if (pieces[0][i].x < 6) {
				break;
			}
		}
		for (int i = 0; ; i++) {
			if (i == pieces[1].size()) {
				return 1;
			}
			else if (pieces[1][i].x > 1) {
				break;
			}
		}
		return 0;
	}
	void show_turn() {

		cout << endl << "-----------------------------------------------------" << endl;
		if (whoseMove) {
			cout << "--------------------White's turn.--------------------" << endl;
		}
		else {
			cout << "--------------------Black's turn.--------------------" << endl;
		}
		cout << "-----------------------------------------------------" << endl << endl;
		cout <<"Turn: "<< moveCount << endl;
		return;
	}

	void move() {
		char chosenDestination;
		int fromX, fromY, toX, toY, pieceIndex;
		visitedSquares.clear();
		Command result;

		show_turn();
		printMap();

		std::chrono::steady_clock::time_point t1 = std::chrono::steady_clock::now();
		// ------------------------------------------------------------------------------------------------------------------------------
		// -------------------------------------------------add evaluation function here-------------------------------------------------
		// ------------------------------------------------------------------------------------------------------------------------------
		if (whoseMove && WHITE_IS_AI) {
			Agent agent(pieces[whoseMove], pieces[!whoseMove], whoseMove);
			agent.work(4, moveCount);
			agent.findBestPath();
			result = agent.get_command();
		}
		else if(!whoseMove && BLACK_IS_AI){
			Agent agent(pieces[whoseMove], pieces[!whoseMove], whoseMove);
			agent.work(4, moveCount);
			agent.findBestPath();
			result = agent.get_command();
		}
		// ------------------------------------------------------------------------------------------------------------------------------
		// ------------------------------------------------------------------------------------------------------------------------------
		// ------------------------------------------------------------------------------------------------------------------------------
		std::chrono::steady_clock::time_point t2 = std::chrono::steady_clock::now();
		long us = long(std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1).count());
		std::cout << "Took " << us << "us, " << us / 1000000 << "s" << endl;
		if (us / 1000000 > 4) {
			int i;
			for (i = 0; i < 100; i++) {
				cout << "exceed time limit. ";
			}
		}

		if ((!whoseMove && BLACK_IS_AI) || (whoseMove && WHITE_IS_AI)) {
			pieceIndex = result.index;
		}
		else {
			pieceIndex = get_piece_index();
		}

		fromX = pieces[whoseMove][pieceIndex].x;
		fromY = pieces[whoseMove][pieceIndex].y;

		int resultIndex = 0;
		while (1) {
			// find the valid movement at this point
			one_step(fromX, fromY, fromX, fromY + 2, 'U');
			one_step(fromX, fromY, fromX + 2, fromY, 'R');
			one_step(fromX, fromY, fromX, fromY - 2, 'D');
			one_step(fromX, fromY, fromX - 2, fromY, 'L');

			// return because of there is no more valid movement.
			if (movements.empty()) {
				cout << "No valid move." << endl;
				moveCount++;
				whoseMove = !whoseMove;
				return;
			}

			printMap();

			// get the choose from player
			toX = -1;
			while (toX < 0) {
				if (visitedSquares.empty()) {
					cout << "Choose the direction (U, R, D, L) on " << pieceIndex << ": ";
				}
				else {
					cout << "Choose the direction or enter X to stop: ";
				}
				if ((!whoseMove && BLACK_IS_AI) || (whoseMove && WHITE_IS_AI)) {
					chosenDestination = result.direction[resultIndex];
					resultIndex++;
				}
				else {
					cin >> chosenDestination;
				}

				if (chosenDestination == 'X') {
					moveCount++;
					whoseMove = !whoseMove;
					movements.clear();
					return;
				}
				for (int i = 0; ; i++) {
					if (i == movements.size()) {
						cout << "This is not a valid choose." << endl;
						break;
					}
					else if (movements[i].direction == chosenDestination) {
						toX = movements[i].x;
						toY = movements[i].y;
						movements.clear();
						break;
					}
				}
			}

			// admit the movement from player
			if (fromX - toX == 1 || toX - fromX == 1 || fromY - toY == 1 || toY - fromY == 1) {
				pieces[whoseMove][pieceIndex].x = toX;
				pieces[whoseMove][pieceIndex].y = toY;
				moveCount++;
				whoseMove = !whoseMove;
				return;
			}
			else if (fromX - toX == 2 || toX - fromX == 2 || fromY - toY == 2 || toY - fromY == 2) {
				int hoppedPieceX = (fromX + toX) / 2, hoppedPieceY = (fromY + toY) / 2, hoppedPieceColor;
				hoppedPieceColor = what_on_this(hoppedPieceX, hoppedPieceY);
				if ((whoseMove == 0 && hoppedPieceColor == -1) || (whoseMove == 1 && hoppedPieceColor == 1)) {
					for (int i = 0; i < pieces[!whoseMove].size(); i++) {
						if (pieces[!whoseMove][i].x == hoppedPieceX && pieces[!whoseMove][i].y == hoppedPieceY) { // eat white or black pieces
							pieces[!whoseMove].erase(pieces[!whoseMove].begin() + i);
							break;
						}
					}
				}
				pieces[whoseMove][pieceIndex].x = toX;
				pieces[whoseMove][pieceIndex].y = toY;
				visitedSquares.push_back({ fromX, fromY });
				fromX = toX;
				fromY = toY;
			}
		}
	}
	int check() {
		int n = 0;
		for (auto piece : pieces[0]) {
			if (piece.x > 5) {
				n++;
			}
		}
		for (auto piece : pieces[1]) {
			if (piece.x < 2) {
				n--;
			}
		}
		if (n > 0) {
			cout << "BLACK WIN!" << endl;
			return 1;
		}
		else if (n == 0) {
			cout << "DRAW!" << endl;
			return 0;
		}
		else {
			cout << "WHITE WIN!" << endl;
			return -1;
		}
	}

private:
	vector<Coordinate> pieces[2];
	bool whoseMove; // 0 for black, 1 for white
	unsigned int moveCount;
	vector<Movement> movements;
	vector<Coordinate> visitedSquares;

	int what_on_this(int x, int y) {
		// -100 for out the board, 1 for black piece, 0 for blank square, -1 for white piece
		if (x < 0 || y < 0 || x > 7 || y > 7) {
			return -100;
		}
		for (auto piece : pieces[0]) {
			if (x == piece.x && y == piece.y) {
				return 1;
			}
		}
		for (auto piece : pieces[1]) {
			if (x == piece.x && y == piece.y) {
				return -1;
			}
		}
		return 0;
	}
	int get_piece_index() {

		int pieceIndex;
		while (1) {
			cout << "Choose your piece: ";
			if (!(cin >> pieceIndex)) {
				cout << "---Invalid choose---" << endl;
				cin.clear();
				cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
				continue;
			}

			if (pieceIndex >= pieces[whoseMove].size() || pieceIndex < 0) {
				cout << "---Invalid choose---" << endl;
			}
			else {
				return pieceIndex;
			}
		}
	}
	void one_step(int fromX, int fromY, int toX, int toY, char dir) {
		if (what_on_this((fromX + toX) / 2, (fromY + toY) / 2) == 0) {
			if (visitedSquares.empty()) {
				movements.push_back(Movement{ (fromX + toX) / 2, (fromY + toY) / 2, dir });
			}
		}
		else if (toX < 8 && toX > -1 && toY < 8 && toY > -1 && what_on_this(toX, toY) == 0) {
			for (int i = 0; ; i++) {
				if (i == visitedSquares.size()) {
					movements.push_back(Movement{ toX, toY, dir });
					break;
				}
				else if (visitedSquares[i].x == toX && visitedSquares[i].y == toY) {
					break;
				}
			}
		}
	}
};

int main() {
	int whiteWin = 0, blackWin = 0;
	for (int i = 0; i < 5; i++) {
		Game game;
		cout << "Game start!" << endl;
		while (!game.isOver()) {
			game.move();
		}
		int w = game.check();
		if (w == 1)
			blackWin++;
		else if (w == -1)
			whiteWin++;
	}

	cout << "black win rate: " << float(blackWin) / 5 << endl;
	cout << "white win rate: " << float(whiteWin) / 5 << endl;
	int intForStop;
	cin >> intForStop;

	return 0;
}