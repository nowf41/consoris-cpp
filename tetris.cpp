// tetris.cpp : Defines the entry point for the application.
//
#define _GLIBCXX_DEBUG

#include "tetris.h"
#include <conio.h>
#include <iostream>
#include <vector>
#include <thread>
#include <chrono>
#include <queue>
#include <algorithm>
#include <numeric>
#include <iterator>
#include <random>

#define rep(i,n) for(int i = 0; i < (n); i++)
#define xfor(i,s,e) for(int i = (s); i < (e); i++)

using namespace std;
void minoTicker();
void keyListener();
void graphicTicker();
namespace Graphic {
	static vector<string> generateString();
	static void doGraph(vector<string> s);
}
namespace Mino {
	vector<vector<vector<string>>> mino = {
		// t
		{
			{
				".#.",
				"###",
				"..."
			},
			{
				".#.",
				".##",
				".#."
			},
			{
				"...",
				"###",
				".#."
			},
			{
				".#.",
				"##.",
				".#."
			}
		},
		// s
		{
			{
				".##",
				"##.",
				"..."
			},
			{
				".#.",
				".##",
				"..#"
			},
			{
				"...",
				".##",
				"##."
			},
			{
				"#..",
				"##.",
				".#."
			}
		},
		// z
		{
			{
				"##.",
				".##",
				"..."
			},
			{
				"..#",
				".##",
				".#."
			},
			{
				"...",
				"##.",
				".##"
			},
			{
				".#.",
				"##.",
				"#.."
			}
		},
		// l
		{
			{
				"..#",
				"###",
				"..."
			},
			{
				".#.",
				".#.",
				".##"
			},
			{
				"...",
				"###",
				"#.."
			},
			{
				"##.",
				".#.",
				".#."
			}
		},
		// j
		{
			{
				"#..",
				"###",
				"..."
			},
			{
				".##",
				".#.",
				".#."
			},
			{
				"...",
				"###",
				"..#"
			},
			{
				".#.",
				".#.",
				"##."
			}
		},
		// i
		{
			{
				"....",
				"####",
				"....",
				"...."
			},
			{
				"..#.",
				"..#.",
				"..#.",
				"..#."
			},
			{
				"....",
				"....",
				"####",
				"...."
			},
			{
				".#..",
				".#..",
				".#..",
				".#.."
			}
		},
		// o
		{
			{
				"##",
				"##"
			}
		}
	};
}

// utils
const string fullLine = "##########";
const string emptyLine = "..........";

// random
random_device seed_gen;
mt19937 engine(seed_gen());

// scoring
int btbScore = 0;
int level = 1;

// game data
bool isEnded = false;
bool isBoardBlocking = false;
unsigned long long score = 0;

vector<string> nowBoard(22, "..........");
auto& getBoard() {
	while (isBoardBlocking) { continue; }
	return nowBoard;
}
auto& getBoard(int i, int j){
	while (isBoardBlocking) { continue; }
	return nowBoard[i][j];
}

int x = 0, y = 3;
int getTickSpeed() {
	if (level == 1) return 1000;
	return pow(800 - ((level - 1) * 7), level-1);
}
int minoKind = engine() % Mino::mino.size(), minoRotation = 0;
bool doHardDrop = false;
bool customGraph = false;
auto getNowMino = [&]() {
	return Mino::mino[minoKind][minoRotation];
};
queue<int> minos;

// debugging
#ifndef release
void debug() {
	while (!isEnded) {
		Graphic::generateString();
	}
}
#endif

int main()
{
	minos.push(rand()%Mino::mino.size());
	thread listenerThread(keyListener);
	thread tickerThread(minoTicker);
	thread graphicThread(graphicTicker);
#ifndef release
	thread dbg(debug);
	dbg.join();
#endif
	
	listenerThread.join();
	tickerThread.join();
	graphicThread.join();

	cout << "GAME OVER!!" << endl;
	system("pause");

	return 0;
}

void graphicTicker() {
	int counter = 0;
	vector<string> nowDisplay(22, "..........");
	while (!isEnded) {
		auto now = Graphic::generateString();
		if (customGraph || now != nowDisplay) 
		{
			customGraph = false;
			system("cls");
			Graphic::doGraph(now);
			nowDisplay = now;
		}
	}
}

void minoTicker()
{
	// TODO implement
	while (!isEnded)
	{
		vector<string> mino = getNowMino();
		// is on the ground?
		auto isOnTheGround = [&mino]() {
			bool res = false;
			rep(i, mino.size()) {
				rep(j, mino[0].size()) {
					if (mino[i][j] == '#' && (x + i + 1 >= 22 || getBoard(x + i + 1, y + j) == '#')) {
						res = true;
					}
				}
			}
			return res;
		};
		
		
		// implementation for hard drop
		if (doHardDrop) while (!isOnTheGround()) x++;

		if  (isOnTheGround()) {
			// put
			rep(i, mino.size()) {
				rep(j, mino[0].size()) {
					if (mino[i][j] == '#') getBoard(i+x, j+y) = '#';
				}
			}
			// erase
			isBoardBlocking = true;
			int erasedLines = 0;
			rep(i, nowBoard.size()) {
				if (nowBoard[i] == fullLine) {
					
					nowBoard.erase(nowBoard.begin() + i);
                    nowBoard.insert(nowBoard.begin(), emptyLine);
					erasedLines++;
				}
			}
			isBoardBlocking = false;

			// do score
			bool isTSpin = false;
			bool isMiniTSpin = false;
			bool isAllClear = false;
			switch (erasedLines) {
			case 0: {
				if (isTSpin) {
					score += 400 * level;
					btbScore += 400 * level;
				}
				else if (isMiniTSpin) {
					score += 100 * level;
					btbScore += 100 * level;
				}
				break;
			}
			case 1: {
				if (isTSpin) {
					int nowScore = 800 * level + (int)((double)0.5 * (double)btbScore);
					score += nowScore;
					btbScore += nowScore;
				}
				else if (isMiniTSpin) {
					int nowScore = 200 * level + (int)((double)0.5 * (double)btbScore);
					score += nowScore;
					btbScore += nowScore;
				}
				else {
					score += 100 * level;
					btbScore = 0;
				}
				break;
			}
			case 2: {
				if (isTSpin) {
					int nowScore = 1200 * level + (int)((double)0.5 * (double)btbScore);
					score += nowScore;
					btbScore += nowScore;
				}
				else {
					score += 300 * level;
					btbScore = 0;
				}
				break;
			}
			case 3: {
				if (isTSpin) {
					int nowScore = 1600 * level + (int)((double)0.5 * (double)btbScore);
					score += nowScore;
					btbScore += nowScore;
				}
				else {
					score += 500 * level;
					btbScore = 0;
				}
				break;
			}
			case 4: {
				int nowScore = 800 * level + (int)((double)0.5 * (double)btbScore);
				score += nowScore;
				btbScore += nowScore;
				break;
			}
			}	
			

			// stop dropping hard
			doHardDrop = false;

			// check game over
			if (nowBoard[2] != emptyLine) {
				isEnded = true;
				break;
			}
			// generate next mino
			if (minos.empty()) {
				vector<int> bundle(Mino::mino.size());
				rep(i, bundle.size()) bundle[i] = i;
				shuffle(bundle.begin(), bundle.end(), engine);
				rep(i, bundle.size()) minos.push(bundle[i]);
			}
			minoKind = minos.front(); // TODO implement bundle
			minos.pop();
			x = 0;
			y = 3;
			minoRotation = 0;
		} else {
			// move down
			x++;
		}
		// regraph
		customGraph = true;

		if (!doHardDrop) {
			this_thread::sleep_for(chrono::milliseconds(getTickSpeed()));
		}
	}
	
}

namespace KeyListeners
{
	static void rotate(bool clockwise) {
		auto nextState = (minoRotation + (clockwise?1:-1)) % Mino::mino[minoKind].size();
		auto nextMino = Mino::mino[minoKind][nextState];
		bool isAble = true;
		rep(i, nextMino.size()) {
			rep(j, nextMino[0].size()) {
				if (nextMino[i][j] == '#') { // if block
					if (!(0 <= i + x && i + x < 22 && 0 <= j + y && j + y < 10)) isAble = false; // もし壁に食い込むなら.
					else if (getBoard(i + x, j + y) == '#') isAble = false;
				}
			}
		}
		if (isAble) minoRotation = nextState;
	}
	static void rotateClockwise()
	{
		rotate(true);
	}

	static void rotateAntiClockwise()
	{
		rotate(false);
	}

	static void hardDrop()
	{
		doHardDrop = true;
	}

	static void hold()
	{
		cout << "hold" << endl;
	}

	static void moveLeft()
	{
		bool hit = false;
		auto now = getNowMino();
		rep(i, now.size()) {
			rep(j, now[0].size()) {
				if (now[i][j] == '#' && (j + y - 1 < 0 || getBoard(i+x, j+y-1) == '#')) {
					hit = true;
				}
			}
		}
		if (!hit) y--;
	}

	static void moveRight()
	{
		// is hit?
		bool hit = false;
		auto now = getNowMino();
		rep(i, now.size()) {
			rep(j, now[0].size()) {
				if (now[i][j] == '#' && (j + y + 1 >= 10 || getBoard(i+x, j+y+1) == '#')) {
					hit = true;
				}
			}
		}
		if (!hit) y++;
	}
};

void keyListener()
{
	while (!isEnded)
	{
		int c;
		bool arrow = false;
		c = _getch();
		if (c == 224)
		{
			arrow = true;
			c = _getch();
		}

		if (arrow)
		{
			switch (c)
			{
			case 'H':
				KeyListeners::rotateClockwise(); // up
				break;
			case 'K':
				KeyListeners::moveLeft(); // left
				break;
			case 'M':
				KeyListeners::moveRight(); // right
				break;
			case 'P':
				break; // down
			}
		}
		else
		{
			switch (c)
			{
			case 'z':
				KeyListeners::rotateAntiClockwise();
				break;
			case 'c':
				KeyListeners::hold();
				break;
			case ' ':
				KeyListeners::hardDrop();
				break;
			}
		}
	}
}

namespace Graphic {
	static vector<string> generateString() {
		auto nextBoardCompared = getBoard();
		vector<string> mino = Mino::mino[minoKind][minoRotation];
		rep(i, mino.size()) {
			rep(j, mino[0].size()) {
				if (mino[i][j] == '#') {
					int newX = x + i;
					int newY = y + j;
					if (newX >= 0 && newX < nextBoardCompared.size() && newY >= 0 && newY < nextBoardCompared[0].size()) {
						nextBoardCompared[newX][newY] = '#';
					}
				}
			}
		}
        char points[11];
        sprintf(points, "%010lld", score);
		nextBoardCompared.push_back("");
        nextBoardCompared.push_back(string(points));
		return nextBoardCompared;
	}
	static void doGraph(vector<string> s) {
		xfor(i, 2, s.size()) {
			rep(j, s[i].size()) {
				cout << s[i][j] << " ";
			}
			cout << endl;
		}
	}
}