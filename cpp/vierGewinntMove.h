#include "move.h"

class VierGewinntMove : public Move {
	public:
	VierGewinntMove() : m_row(-1) {};
	VierGewinntMove(int f_row) : m_row(f_row) {};
	int m_row;
};
