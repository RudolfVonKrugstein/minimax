#include <iostream>
#include "minimax.inl"
#include "vierGewinntBoard.h"
#include <time.h>

int main() {
	VierGewinntBoard l_board;	
	MiniMaxAI<VierGewinntBoard, VierGewinntMove, WIN_SCORE> ai(l_board);
	MiniMaxAI<VierGewinntBoard, VierGewinntMove, WIN_SCORE> aiMemory(l_board);
	aiMemory.enableTranspositionTable(19,0);
	do {
		// Player input
		VierGewinntMove l_turn(-1);
		do {
			l_board.drawToStdout();
			std::cout << "Thinking ..." << std::endl;
			double start = clock();
			l_turn = ai.getAlphaBetaBestMove(12);
			double end = clock();
			int score = ai.getLastMiniMaxScore();
			std::cout << "(No Memory) Putting token into " << l_turn.m_row << " expecting score of " << score << ". It took " << end-start << " ticks." << std::endl;
			start = clock();
			l_turn = aiMemory.getAlphaBetaBestMove(12);
			end = clock();
			score = ai.getLastMiniMaxScore();
			std::cout << "(With Memory) Putting token into " << l_turn.m_row << " expecting score of " << score << ". It took " << end-start << " ticks." << std::endl;
			l_board.applyMove(l_turn);
			if (l_board.didPlayerWin()) break;
			l_board.drawToStdout();
			std::cout << "Select row to insert token:";
			char row;
			std::cin >> row;
			std::cin.clear();
			switch (row) {
				case '0':
					l_turn.m_row = 0;
					break;
				case '1':
					l_turn.m_row = 1;
					break;
				case '2':
					l_turn.m_row = 2;
					break;
				case '3':
					l_turn.m_row = 3;
					break;
				case '4':
					l_turn.m_row = 4;
					break;
				case '5':
					l_turn.m_row = 5;
					break;
				case '6':
					l_turn.m_row = 6;
					break;
			}
		} while(l_turn.m_row == -1);
		l_board.applyMove(l_turn);
		if (l_board.didPlayerWin()) break;
	} while(l_board.turnsLeft());
	l_board.drawToStdout();
	if (l_board.getScore() == -WIN_SCORE) std::cout << "You WON!!!"<<std::endl; else
		if (l_board.getScore() == WIN_SCORE) std::cout << "You LOSE!!!"<<std::endl; else
			std::cout << "Its a tie"<<std::endl;
}
