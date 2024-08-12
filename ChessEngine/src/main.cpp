#include <iostream>
#include "board.h"
#include "move.h"
#include "../../utils/utils.h"
#include <sys/time.h>

int get_time_ms()
{
    struct timeval time_value;
    gettimeofday(&time_value, NULL);

    return time_value.tv_sec * 1000 + time_value.tv_usec / 1000;
}

int main() {
    Board board;
    board.set_from_fen("r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1");

    int iter = 1000;
    int percent = 0;

    int start = get_time_ms();
    for (int i = 0; i < iter; i++)
    {

        if (i % (iter / 10) == 0)
        {
            percent++;
            std::cout << "Progress: " << percent * 10 << "%" << std::endl;
        }

        board.legal_moves_calculated = false;
        board.legal_moves.clear();
        board.get_all_legal_moves();
    }
    int end = get_time_ms();

    std::cout << "Time taken to calculate legal moves in " << iter << " position: " << end - start << " ms" << std::endl; 

}