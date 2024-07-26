#include "utils.h"

bool isupper(char c) {
    return c >= 'A' && c <= 'Z';
}

int find_nth(std::string s, char c, int n){
    int count = 0;
    for (int i=0; i<s.length(); i++){
        if(s[i] == c){
            count++;
        }
        if (count>=n){
            return i;
        }
    }
    return -1;
}

std::string coordinates_to_square(int row, int col){
    std::string s; s += (char)('a' + col); s += (char)('8' - (row));
    return s;
}
Square_Coordinates square_to_coordinates(std::string square){
    Square_Coordinates sc;

    sc.row = 8 - (square[1] - '0');
    sc.col = square[0] - 'a';

    return sc;
}