#include <iostream>
#include <vector>
#include <limits>
#include <thread>
#include <mutex>

// feature: thread, alpha-beta pruning

using namespace std;


struct Coordinate {
	int x, y;
};

struct PathInfo {
	vector<Coordinate> path;
	int value;
	vector<int> eatingHops;
};

struct Command {
	vector<char> direction;
	int index;
};

mutex fmutex;

class Agent {
public:
	Agent(vector<Coordinate> myPiece, vector<Coordinate> oppenentsPiece, bool color) {
		pieces[0] = myPiece;
		pieces[1] = oppenentsPiece;
		myColor = color;
	};
	void work(int howManyMove, int moveCount) {
		if (moveCount == 0) {
			finalMovements.push_back(PathInfo{ vector<Coordinate>{ {0, 7} ,{ 1,7 }} ,0 });
			return;
		}
		else if (moveCount == 1) {
			finalMovements.push_back(PathInfo{ vector<Coordinate>{ {7, 6} ,{ 6,6 }} ,0 });
			return;
		}
		else if (moveCount == 2) {
			finalMovements.push_back(PathInfo{ vector<Coordinate>{ {0, 1} ,{ 0,2 }} ,0 });
			return;
		}
		else if (moveCount == 3) {
			finalMovements.push_back(PathInfo{ vector<Coordinate>{ {7,0} ,{ 7,1 }} ,0 });
			return;
		}

		if (moveCount < 20) {
			howManyMove -= 2;
		}

		if (pieces[0].size() + pieces[1].size() < 7) {
			howManyMove += 2;
		}

		vector<thread> threadDigs;
		for (int i = 1; i < pieces[0].size(); i++) {
			threadDigs.push_back(thread(&Agent::thread_dig, this, howManyMove - 1, i));
		}
		thread_dig(howManyMove - 1, 0);

		for (int i = 0; i < threadDigs.size(); i++) {
			if (threadDigs[i].joinable()) {
				threadDigs[i].join();
			}
		}
	}
	int propagate_back() {
		int max = 0;
		for (auto path : finalMovements) {
			if (max < path.value) {
				max = path.value;
			}
		}
		return max;
	}
	void findBestPath() {
		int max = -100;
		for (auto path : finalMovements) {
			if (max < path.value) {
				cout << "value: " << path.value << endl;
				max = path.value;
				for (auto c : path.path) {
					cout << "(" << c.x << ", " << c.y << ") ";
				}
				cout << endl;
			}
		}
	}
	Command get_command() {
		Command c;
		int maxOne = 0;
		for (int i = 0; i < finalMovements.size(); i++) {
			if (finalMovements[maxOne].value < finalMovements[i].value) {
				maxOne = i;
			}
		}

		for (int i = 0; i < pieces[0].size(); i++) {
			if (finalMovements[maxOne].path[0].x == pieces[0][i].x && finalMovements[maxOne].path[0].y == pieces[0][i].y) {
				c.index = i;
				break;
			}
		}

		for (int j = 0; j < finalMovements[maxOne].path.size() - 1; j++) {
			if (finalMovements[maxOne].path[j].x - finalMovements[maxOne].path[j + 1].x > 0) {
				c.direction.push_back('L');
			}
			else if (finalMovements[maxOne].path[j].x - finalMovements[maxOne].path[j + 1].x < 0) {
				c.direction.push_back('R');
			}
			else if (finalMovements[maxOne].path[j].y - finalMovements[maxOne].path[j + 1].y > 0) {
				c.direction.push_back('D');
			}
			else {
				c.direction.push_back('U');
			}
		}
		c.direction.push_back('X');

		return c;
	}

private:
	vector<Coordinate> pieces[2];
	vector<PathInfo> finalMovements, unfinishedMovements;
	bool myColor; // 0 for black, 1 for white
	inline int what_on_this(int x, int y) {
		// !!!!!!!!!!!!! 1 for mine, -1 for oppenent's !!!!!!!!!!!!!!!!!!!!
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
	inline int win_or_lose(vector<Coordinate> &M, vector<Coordinate> &O) {
		int n = 0, piecesDifference = int(M.size()) - int(O.size());
		if (piecesDifference > 0) {
			n = 100;
		}
		else if (piecesDifference < 0) {
			n = -100;
		}

		for (int i = 0; ; i++) {
			if (i == M.size()) {
				return n;
			}
			else if (M[i].x < 6) {
				break;
			}
		}
		for (int i = 0; ; i++) {
			if (i == O.size()) {
				return n;
			}
			else if (O[i].x > 1) {
				break;
			}
		}
		return 0;
	}
	void thread_one_step(vector<PathInfo> &fm, vector<PathInfo> &um, int fromX, int fromY, int walkToX, int walkToY, int hopToX, int hopToY) {
		int what = what_on_this(walkToX, walkToY);
		if (what == 0) {
			fm.push_back(PathInfo{ vector<Coordinate>{ { fromX, fromY },{ walkToX, walkToY }}, [=](bool c) {if (c) return (fromX - walkToX); else return (walkToX - fromX); }(myColor) });
		}
		else if (hopToX < 8 && hopToX > -1 && hopToY < 8 && hopToY > -1 && what_on_this(hopToX, hopToY) == 0) {
			um.push_back(PathInfo{ vector<Coordinate> { { fromX, fromY},{ hopToX, hopToY }}, [=](bool c) {if (c) return (fromX - hopToX); else return (hopToX - fromX); }(myColor) * 3 / 2 });
			if (what == -1) {
				um.back().value += 10;
				um.back().eatingHops.push_back(int(um.back().path.size()) - 1);
			}
		}
	}
	void thread_hop_again(vector<PathInfo> &um, vector<PathInfo> &fm, int fromX, int fromY, int toX, int toY, int pathSize, PathInfo const &path) {
		int what = what_on_this((fromX + toX) / 2, (fromY + toY) / 2);
		if (what != 0 && toX < 8 && toX > -1 && toY < 8 && toY > -1 && what_on_this(toX, toY) == 0) {
			for (int i = 0; ; i++) {
				if (i == pathSize) {
					PathInfo temp = path;
					temp.path.push_back({ toX, toY });
					if (what == -1) {
						temp.value += 10;
						temp.eatingHops.push_back(pathSize);
					}
					temp.value += [=](bool c) {if (c) return (fromX - toX); else return (toX - fromX); }(myColor) * 3 / 2;
					for (int j = 0; ; j++) {
						if (j == fm.size()) {
							um.push_back(temp);
							break;
						}
						else if (temp.path.front().x == fm[j].path.front().x && temp.path.front().y == fm[j].path.front().y && temp.path.back().x == fm[j].path.back().x && temp.path.back().y == fm[j].path.back().y && temp.eatingHops == fm[j].eatingHops) {
							break;
						}
					}
					break;
				}
				else if (path.path[i].x == toX && path.path[i].y == toY) {
					break;
				}
			}
		}
	}
	void thread_dig(int depth, int pieceIndex) {
		int fromX = pieces[0][pieceIndex].x, fromY = pieces[0][pieceIndex].y, hoppedInfo;
		vector<PathInfo> threadFinalMovements, threadUnfinishedMovements;

		// handle the one step movements
		thread_one_step(threadFinalMovements, threadUnfinishedMovements, fromX, fromY, fromX, fromY + 1, fromX, fromY + 2);
		thread_one_step(threadFinalMovements, threadUnfinishedMovements, fromX, fromY, fromX, fromY - 1, fromX, fromY - 2);
		thread_one_step(threadFinalMovements, threadUnfinishedMovements, fromX, fromY, fromX + 1, fromY, fromX + 2, fromY);
		thread_one_step(threadFinalMovements, threadUnfinishedMovements, fromX, fromY, fromX - 1, fromY, fromX - 2, fromY);

		// handle the hopping movement
		while (!threadUnfinishedMovements.empty()) {
			PathInfo gloablPath = threadUnfinishedMovements.back();
			threadUnfinishedMovements.pop_back();
			threadFinalMovements.push_back(gloablPath);
			int pathSize = int(gloablPath.path.size());

			fromX = gloablPath.path.back().x;
			fromY = gloablPath.path.back().y;

			thread_hop_again(threadUnfinishedMovements, threadFinalMovements, fromX, fromY, fromX, fromY + 2, pathSize, gloablPath);
			thread_hop_again(threadUnfinishedMovements, threadFinalMovements, fromX, fromY, fromX, fromY - 2, pathSize, gloablPath);
			thread_hop_again(threadUnfinishedMovements, threadFinalMovements, fromX, fromY, fromX + 2, fromY, pathSize, gloablPath);
			thread_hop_again(threadUnfinishedMovements, threadFinalMovements, fromX, fromY, fromX - 2, fromY, pathSize, gloablPath);
		}

		for (int i = 0; i < threadFinalMovements.size(); i++) {
			vector<Coordinate> myPieces = pieces[0], opponentPieces;
			int j;
			for (j = 0; j < myPieces.size(); j++) {
				if (myPieces[j].x == threadFinalMovements[i].path[0].x && myPieces[j].y == threadFinalMovements[i].path[0].y) {
					myPieces[j].x = threadFinalMovements[i].path.back().x;
					myPieces[j].y = threadFinalMovements[i].path.back().y;
					break;
				}
			}
			j = 0;
			for (auto p : pieces[1]) {
				if (j < threadFinalMovements[i].eatingHops.size() && p.x == (threadFinalMovements[i].path[threadFinalMovements[i].eatingHops[j]].x + threadFinalMovements[i].path[threadFinalMovements[i].eatingHops[j] - 1].x) / 2 && p.y == (threadFinalMovements[i].path[threadFinalMovements[i].eatingHops[j]].y + threadFinalMovements[i].path[threadFinalMovements[i].eatingHops[j] - 1].y) / 2) {
					j++;
					continue;
				}
				opponentPieces.push_back(p);
			}
			Agent opp(opponentPieces, myPieces, !myColor);
			opp.dig(depth - 1, -100);
			threadFinalMovements[i].value -= opp.propagate_back();
		}
		fmutex.lock();
		for (auto tfm : threadFinalMovements) {
			finalMovements.push_back(tfm);
		}
		fmutex.unlock();
	}
	bool one_step(int fromX, int fromY, int walkToX, int walkToY, int hopToX, int hopToY, int depth, int alpha) {
		int what = what_on_this(walkToX, walkToY);
		if (what == 0) {
			finalMovements.push_back(PathInfo{ vector<Coordinate>{ { fromX, fromY },{ walkToX, walkToY }}, [=](bool c) {if (c) return (fromX - walkToX); else return (walkToX - fromX); }(myColor) });
			if (depth == 0 && alpha > finalMovements.back().value) {
				return 1;
			}
		}
		else if (hopToX < 8 && hopToX > -1 && hopToY < 8 && hopToY > -1 && what_on_this(hopToX, hopToY) == 0) {
			unfinishedMovements.push_back(PathInfo{ vector<Coordinate> { { fromX, fromY},{ hopToX, hopToY }}, [=](bool c) {if (c) return (fromX - hopToX); else return (hopToX - fromX); }(myColor) * 3 / 2 });
			if (what == -1) {
				unfinishedMovements.back().value += 10;
				unfinishedMovements.back().eatingHops.push_back(int(unfinishedMovements.back().path.size()) - 1);
				if (depth == 0 && alpha > unfinishedMovements.back().value) {
					return 1;
				}
			}
		}
		return 0;
	}
	bool hop_again(int fromX, int fromY, int toX, int toY, int pathSize, PathInfo const &path, int depth, int alpha) {
		int what = what_on_this((fromX + toX) / 2, (fromY + toY) / 2);
		if (what != 0 && toX < 8 && toX > -1 && toY < 8 && toY > -1 && what_on_this(toX, toY) == 0) {
			for (int i = 0; ; i++) {
				if (i == pathSize) {
					PathInfo temp = path;
					temp.path.push_back({ toX, toY });
					if (what == -1) {
						temp.value += 10;
						temp.eatingHops.push_back(pathSize);
					}
					temp.value += [=](bool c) {if (c) return (fromX - toX); else return (toX - fromX); }(myColor) * 3 / 2;
					for (int j = 0; ; j++) {
						if (j == finalMovements.size()) {
							unfinishedMovements.push_back(temp);
							if (depth == 0 && alpha > finalMovements.back().value) {
								return 1;
							}
							break;
						}
						else if (temp.path.front().x == finalMovements[j].path.front().x && temp.path.front().y == finalMovements[j].path.front().y && temp.path.back().x == finalMovements[j].path.back().x && temp.path.back().y == finalMovements[j].path.back().y && temp.eatingHops == finalMovements[j].eatingHops) {
							break;
						}
					}
					break;
				}
				else if (path.path[i].x == toX && path.path[i].y == toY) {
					break;
				}
			}
		}
		return 0;
	}
	void dig(int depth, int alpha) {
		for (auto piece : pieces[0]) {
			int fromX = piece.x, fromY = piece.y, hoppedInfo;

			// handle the one step movements
			if (one_step(fromX, fromY, fromX, fromY + 1, fromX, fromY + 2, depth, alpha) ||
			one_step(fromX, fromY, fromX, fromY - 1, fromX, fromY - 2, depth, alpha) ||
			one_step(fromX, fromY, fromX + 1, fromY, fromX + 2, fromY, depth, alpha) ||
			one_step(fromX, fromY, fromX - 1, fromY, fromX - 2, fromY, depth, alpha)) {
				return;
			}

			// handle the hopping movement
			while (!unfinishedMovements.empty()) {
				PathInfo gloablPath = unfinishedMovements.back();
				unfinishedMovements.pop_back();
				finalMovements.push_back(gloablPath);
				int pathSize = int(gloablPath.path.size());

				fromX = gloablPath.path.back().x;
				fromY = gloablPath.path.back().y;

				if (hop_again(fromX, fromY, fromX, fromY + 2, pathSize, gloablPath, depth, alpha) ||
				hop_again(fromX, fromY, fromX, fromY - 2, pathSize, gloablPath, depth, alpha) ||
				hop_again(fromX, fromY, fromX + 2, fromY, pathSize, gloablPath, depth, alpha) ||
				hop_again(fromX, fromY, fromX - 2, fromY, pathSize, gloablPath, depth, alpha)) {
					return;
				}
			}
		}
		
		// spread it
		for (int i = 0; i < finalMovements.size(); i++) {
			vector<Coordinate> myPieces = pieces[0], opponentPieces;
			for (int j = 0; j < myPieces.size(); j++) {
				if (myPieces[j].x == finalMovements[i].path[0].x && myPieces[j].y == finalMovements[i].path[0].y) {
					myPieces[j].x = finalMovements[i].path.back().x;
					myPieces[j].y = finalMovements[i].path.back().y;
					break;
				}
			}
			int j = 0;
			for (auto p : pieces[1]) {
				if (j < finalMovements[i].eatingHops.size() && p.x == (finalMovements[i].path[finalMovements[i].eatingHops[j]].x + finalMovements[i].path[finalMovements[i].eatingHops[j] - 1].x) / 2 && p.y == (finalMovements[i].path[finalMovements[i].eatingHops[j]].y + finalMovements[i].path[finalMovements[i].eatingHops[j] - 1].y) / 2) {
					j++;
					continue;
				}
				opponentPieces.push_back(p);
			}

			finalMovements[i].value += win_or_lose(myPieces, opponentPieces);
			if (depth != 0) {
				Agent opp(opponentPieces, myPieces, !myColor);
				opp.dig(depth - 1, alpha - finalMovements[i].value);
				finalMovements[i].value -= opp.propagate_back();
				if (alpha < finalMovements[i].value) {
					alpha = finalMovements[i].value;
				}
			}
		}
	}
};
