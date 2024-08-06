#include <iostream>
#include "board.h"

std::tuple<std::string, double> minmax(Board board, int depth, bool maximizing_player, double alpha, double beta);
double evaluate(Board& board);
double piece_value(int piece);
