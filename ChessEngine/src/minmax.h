#include <iostream>
#include "board.h"

struct minmax_result{
    std::string move;
    double evaluation;
};
minmax_result minmax(Board board, int depth, bool maximizing_player, double alpha, double beta);
double evaluate(Board& board);
double piece_value(int piece);
