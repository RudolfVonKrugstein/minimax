#include <stdexcept>

class TranspositionTable {
	public:
		typedef long long ZobristKey;
		enum EntryType {
			TTE_EXACT,
			TTE_LOWERBOUND,
			TTE_UPPERBOUND
		};
		struct Entry {
			ZobristKey m_zobristKey;
			int m_value;	
			EntryType m_type;
			char m_depth;
			short m_lastAccessTime;
			short m_creationTime;
		};
		TranspositionTable(const size_t fc_size);
		~TranspositionTable();
		void changeSize(const size_t fc_size);

		const Entry* getEntry(const ZobristKey fc_zobristKey, const int fc_depth, const short fc_currentTime);
		void tryStoreEntry(const ZobristKey fc_zobristKey, const int fc_depth, const int fc_bestScore, const EntryType fc_type, const short fc_currentTime);
	private:
		size_t m_size;
		size_t m_accessBits;
		Entry* m_table;
};

TranspositionTable::TranspositionTable(const size_t fc_size) {
	m_table = 0;
	changeSize(fc_size);
}

TranspositionTable::~TranspositionTable() {
	if (m_table) {
		delete [] m_table;
	}
	m_table = NULL;
	m_size = 0;
}

void TranspositionTable::changeSize(const size_t fc_size) {
	if (m_table) {
		delete [] m_table;
	}
	if (fc_size == 0) {
		m_size = 0;
		m_table = 0;
		return;
	}
	m_size = 1 << fc_size;
	m_table = new Entry[m_size];
	if (!m_table) {
		m_size = 0;
		throw std::runtime_error("Not enough space for transposition table.");
	}
	for (size_t i = 0; i < m_size; ++i) {
		m_table[i].m_lastAccessTime = -1;
		m_table[i].m_creationTime  = -1;
	}
	m_accessBits = m_size - 1;
}

const TranspositionTable::Entry* TranspositionTable::getEntry(const ZobristKey f_zobristKey, const int fc_depth, const short fc_currentTime) {
	size_t l_index = f_zobristKey & m_accessBits;
	if (m_table && m_table[l_index].m_depth >= fc_depth && m_table[l_index].m_zobristKey == f_zobristKey) {
		m_table[l_index].m_lastAccessTime = fc_currentTime;
		return &m_table[l_index];
	}
	return 0;
}

void TranspositionTable::tryStoreEntry(const ZobristKey fc_zobristKey, const int fc_depth, const int fc_bestScore, const EntryType fc_type, const short fc_currentTime) {
	if (!m_table) return;
	size_t l_index = fc_zobristKey & m_accessBits;
	Entry& l_entry = m_table[l_index];
	// If used in this round, do not overwrite!
	if (l_entry.m_lastAccessTime == fc_currentTime) return;
	
	// Store the entry
	l_entry.m_zobristKey = fc_zobristKey;
	l_entry.m_depth = fc_depth;
	l_entry.m_value = fc_bestScore;
	l_entry.m_type = fc_type;
	l_entry.m_creationTime = fc_currentTime;
	l_entry.m_lastAccessTime = -1;
}

template<class Board, class Move, int WINSCORE>
class MiniMaxAI {
	public:
		MiniMaxAI(Board& fr_board);
		/** Return the best move of an alpha-beta search.
		 *  The corresponding minimax score can be retrieved with getLastMiniMaxScore().
		 *  @param f_depth The search depth to search. The real depth also is effected by the return value of Move.usedDepth().
		 *  @return The best move from an alpha-beta minimax search.
		 */
		Move getAlphaBetaBestMove(const int f_depth);
		
		int getLastMiniMaxScore(){return m_lastScore;}

		void enableTranspositionTable(const size_t fc_size, const int fc_tableDepth);

	private:
		/** Return the alpha-beta (minimax with alpha beta pruning) score, implementation with all details.
		 *  This function is normaly not directly accesed, but through others, i.E. getAlphaBetaBestMove.
		 *  @param f_depth The search depth to search. The real depth also is effected by the return value of Move.usedDepth().
		 *  @param fp_bestMove Pointer to store the best found move into. If no move should be stored, set to NULL.
		 *  @param f_alpha Initial alpha value (lower bound) of the expected score.
		 *  @param f_beta Initial beta value (upper bound) of the expected score.
		 *  @return The minimax score of the search, but only if it is between f_alpha and f_beta.
		 */
		int getAlphaBetaScore(const int f_depth, Move* f_storeBestMove, int f_alpha, int f_beta);

		// The board we are working on
		Board& mr_board;

		// Last minimax score
		int m_lastScore;

		short m_turnCount;

		// How deep before the leaf nodes should the transposition table be used?
		int m_transpositionTableDepth;

		TranspositionTable m_transpositionTable;
};

template<class Board, class Move, int WINSCORE>
MiniMaxAI<Board,Move,WINSCORE>::MiniMaxAI(Board& fr_board) : mr_board(fr_board), m_turnCount(0), m_transpositionTableDepth(1000), m_transpositionTable(0) {}

template<class Board, class Move, int WINSCORE>
Move MiniMaxAI<Board,Move,WINSCORE>::getAlphaBetaBestMove(const int f_depth) {
	Move l_result;
	m_lastScore = getAlphaBetaScore(f_depth, &l_result, -WINSCORE,WINSCORE);
	++m_turnCount;
	if (m_turnCount < 0) {
		m_turnCount = 0;
	}
	return l_result;
}

template<class Board, class Move, int WINSCORE>
void MiniMaxAI<Board,Move,WINSCORE>::enableTranspositionTable(const size_t fc_size, const int fc_tableDepth) {
	m_transpositionTable.changeSize(fc_size);
	m_transpositionTableDepth = fc_tableDepth;
}

template<class Board, class Move, int WINSCORE>
int MiniMaxAI<Board,Move,WINSCORE>::getAlphaBetaScore(const int f_depth, Move* fp_bestMove, int f_alpha, int f_beta) {
	// Test for win situation
	if (mr_board.didPlayerWin())
		return mr_board.getScore();

	// End of depth
	if (f_depth <= 0) {
		return mr_board.getScore();
	}
	// Transposition table lookup
	if (m_transpositionTableDepth <= f_depth) {
		const TranspositionTable::Entry* l_entry = m_transpositionTable.getEntry(mr_board.getZobristKey(), f_depth, m_turnCount);
		if (l_entry != 0) { // Check if an entry has been found
			switch(l_entry->m_type) {
				case TranspositionTable::TTE_EXACT:
					return l_entry->m_value;
				case TranspositionTable::TTE_LOWERBOUND:
					if (l_entry->m_value > f_alpha) f_alpha = l_entry->m_value;
					break;
				case TranspositionTable::TTE_UPPERBOUND:
					if (l_entry->m_value < f_beta) f_beta = l_entry->m_value;
					break;
			}
			if (f_alpha >= f_beta) return l_entry->m_value; // Beta cut off from table entry
		}

	}
	// iterate over all turns
	Move l_move;
	int l_bestScore = -WINSCORE-1; // Initilize lower than any possible score
	int l_origAlpha = f_alpha;     // Save alpha for transposition table
	if (!mr_board.firstMove(l_move)) return 0; //Tie
	do{
		typename Board::MoveUndoInfo l_undoInfo;
		mr_board.applyMove(l_move, l_undoInfo);
		int score;
		if (l_move.switchesActivePlayer()) {
			// Normal negamax branch
			score = -getAlphaBetaScore(f_depth - l_move.usedDepth(),0, -f_beta, -f_alpha);
		} else {
			score = getAlphaBetaScore(f_depth - l_move.usedDepth(),0, f_alpha, f_beta);
		}
		mr_board.reverseMove(l_move, l_undoInfo);
		// Test for beta score
		if (score > l_bestScore) {
			l_bestScore = score;
			if (fp_bestMove) *fp_bestMove = l_move;
		}
		// Update alpha
		if (score > f_alpha)
			f_alpha = score;
		// Test for beta cut off
		if (score >= f_beta)
			break;
	} while(mr_board.getNextMove(l_move));
	// Store in transposition table
	if (m_transpositionTableDepth <= f_depth) {
		if (l_bestScore <= l_origAlpha) {
			m_transpositionTable.tryStoreEntry(mr_board.getZobristKey(), f_depth, l_bestScore, TranspositionTable::TTE_UPPERBOUND, m_turnCount);
		} else if (l_bestScore >= f_beta) {
			m_transpositionTable.tryStoreEntry(mr_board.getZobristKey(), f_depth, l_bestScore, TranspositionTable::TTE_LOWERBOUND, m_turnCount);
		} else {
			m_transpositionTable.tryStoreEntry(mr_board.getZobristKey(), f_depth, l_bestScore, TranspositionTable::TTE_EXACT, m_turnCount);
		}
	}
	return f_alpha;
}
