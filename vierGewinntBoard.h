#include "vierGewinntMove.h"
#include <limits.h>
#include <iostream>
#include <stdlib.h>

#define WIN_SCORE 1000

class VierGewinntBoard
{
	public:
		VierGewinntBoard();
		typedef int MoveUndoInfo;
		typedef VierGewinntMove Move;
		bool didPlayerWin();
		int getScore();
		bool firstMove(Move& fr_turn);
		void applyMove(const Move& fr_turn);
		void applyMove(const Move& fr_turn, MoveUndoInfo& undo);
		void reverseMove(const Move& fr_turn, const MoveUndoInfo undo);
		bool getNextMove(Move& fr_turn);
		void drawToStdout();
		bool turnsLeft();
		long long getZobristKey() {return m_zobristKey;};

	private:
		// The board itself
		enum FIELD_TYPE{
			EMPTY,
			PLAYER1,
			PLAYER2
		};
		FIELD_TYPE m_field[7][6];
		// Addition information
		int m_rowHeights[7];
		bool m_player1Turn;
		
		// For the KI, the score out of the view of player one
		int m_kiScore;

		inline bool insideBoardPosition(int f_testX, int f_testY);
		inline void updateScore(const int f_testX, const int f_testY, const int f_dx,const int f_dy, const FIELD_TYPE f_testColor, const int f_playerFactor);

		// current zobrist key
		long long m_zobristKey;
		long long m_zKeys[7][6][2];

};

bool VierGewinntBoard::turnsLeft() {
	for (unsigned int i = 0; i < 7; ++i) {
		if (m_rowHeights[i] <5) {
			return true;
		}
	}
	return false;
}

bool VierGewinntBoard::insideBoardPosition(int f_testX, int f_testY) {
	return f_testX >= 0 && f_testY >= 0 && f_testX < 7 && f_testY < 6;
}

/** Update score when a new token is inserted at position f_testX, f_testY.
 *  The score is defined as:
 *  1 Point for every 2token row with one open end
 *  2 Points for every 2token row with two open end
 *  10 Point for every 3token row with one open end
 *  20 Points for every 3token row with two open end
 */
void VierGewinntBoard::updateScore(const int f_testX, const int f_testY, const int f_dx,const int f_dy, const FIELD_TYPE f_testColor, const int f_playerFactor) {
	if (m_kiScore == WIN_SCORE || m_kiScore == -WIN_SCORE) return;
	int factor = 0;
	int count1 = -1;
	int x = f_testX; int y = f_testY;
	// Count the number of this color in the one direction
	do {
		x -= f_dx;;
		y -= f_dy;
		++count1;
	} while(insideBoardPosition(x, y) && m_field[x][y] == f_testColor);
	// Test if there is a free field
	if (insideBoardPosition(x, y) && m_field[x][y] == EMPTY) ++factor;
	// Same in the other direction
	int count2 = -1;
	// Count the number of this color in the one direction
	x = f_testX; y = f_testY;
	do {
		x += f_dx;
		y += f_dy;
		++count2;
	} while(insideBoardPosition(x, y) && m_field[x][y] == f_testColor);
	// Test if there is a free field
	if (insideBoardPosition(x, y) && m_field[x][y] == EMPTY) ++factor;
	// Reduce score by previous rows
	if (count1 == 2) m_kiScore -= f_playerFactor;
	if (count2 == 2) m_kiScore -= f_playerFactor;
	// Add score for new row
	if (count1 + count2 + 1 == 2) m_kiScore += factor * f_playerFactor;
	if (count1 + count2 + 1 == 3) m_kiScore += 10 * factor * f_playerFactor;
	if (count1 + count2 + 1 >= 4) if (m_player1Turn) m_kiScore = WIN_SCORE; else  m_kiScore = -WIN_SCORE;
}

VierGewinntBoard::VierGewinntBoard() : m_player1Turn(true), m_kiScore(0), m_zobristKey(0) {
	for (unsigned int i = 0; i < 7; ++i) {
		for (unsigned int j = 0; j < 6; ++j) {
			m_field[i][j] = EMPTY;
			m_zKeys[i][j][0] = rand()^((long long)rand()<<15)^((long long)rand()<<30)^((long long)rand()<<45)^((long long)rand()<<60);
			m_zKeys[i][j][1] = rand()^((long long)rand()<<15)^((long long)rand()<<30)^((long long)rand()<<45)^((long long)rand()<<60);
		}
		m_rowHeights[i] = 0;
	}
}

bool VierGewinntBoard::didPlayerWin() {
	return m_kiScore == WIN_SCORE || m_kiScore == -WIN_SCORE;
}

int VierGewinntBoard::getScore() {
	return (m_player1Turn)?m_kiScore:-m_kiScore;
}

bool VierGewinntBoard::firstMove(Move& fr_move) {
	for (fr_move.m_row = 0; fr_move.m_row < 7; ++fr_move.m_row) {
		if (m_rowHeights[fr_move.m_row] < 5) {
			return true;
		}
	}
	return false;
}
void VierGewinntBoard::applyMove(const Move& fr_turn) {
	MoveUndoInfo undo;
	applyMove(fr_turn, undo);
}

void VierGewinntBoard::applyMove(const Move& fr_turn, MoveUndoInfo& undo) {
	// undo information
	undo = m_kiScore;
	FIELD_TYPE l_color = m_player1Turn?PLAYER1:PLAYER2;
	// Calculate new score
	int testX = fr_turn.m_row;
	int testY = m_rowHeights[fr_turn.m_row];
	int f_playerFactor = m_player1Turn?1:-1;
	updateScore(testX, testY, 1, 0, l_color, f_playerFactor);
	updateScore(testX, testY, 1, 1, l_color, f_playerFactor);
	updateScore(testX, testY, 0, 1, l_color, f_playerFactor);
	updateScore(testX, testY, 1, -1, l_color, f_playerFactor);
	// Update zobrist key
	m_zobristKey ^= m_zKeys[testX][testY][(int)m_player1Turn];
	// Set the field
	m_field[testX][testY] = l_color;
	++m_rowHeights[fr_turn.m_row];
	m_player1Turn = !m_player1Turn;
}

void VierGewinntBoard::reverseMove(const Move& fr_turn, const MoveUndoInfo undo) {
	// undo information
	m_kiScore = undo;
	//Set the field
	int testX = fr_turn.m_row;
	--m_rowHeights[fr_turn.m_row];
	int testY = m_rowHeights[fr_turn.m_row];
	m_field[testX][testY] = EMPTY;
	m_player1Turn = !m_player1Turn;
	// Update zobrist key
	m_zobristKey ^= m_zKeys[testX][testY][(int)m_player1Turn];
}

bool VierGewinntBoard::getNextMove(Move& fr_turn) {
	for (++fr_turn.m_row; fr_turn.m_row < 7; ++fr_turn.m_row) {
		if (m_rowHeights[fr_turn.m_row] < 5) {
			return true;
		}
	}
	return false;
}


void VierGewinntBoard::drawToStdout() {
	std::cout << "| 0 | 1 | 2 | 3 | 4 | 5 | 6 |" << std::endl;
	std::cout << "=============================" << std::endl;
	for (unsigned int i = 0; i < 6; ++i) {
		std::cout << "|";
		for (unsigned int j = 0; j < 7; ++j) {
			switch(m_field[j][5-i]) {
				case EMPTY:
					std::cout << "   |";
					break;
				case PLAYER1:
					std::cout << " X |";
					break;
				case PLAYER2:
					std::cout << " O |";
					break;
			}
		}
		std::cout << std::endl << "-----------------------------" << std::endl;
	}
	std::cout << "|";
	for (unsigned int j = 0; j < 7; ++j) {
		std::cout << "["<<m_rowHeights[j]<<"]|";
	}
	std::cout << std::endl << "=============================" << std::endl;
	std::cout << "Score: " << m_kiScore << std::endl;
}
