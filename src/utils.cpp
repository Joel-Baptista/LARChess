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

bool is_in_ray(std::string from, std::string to, std::string square){
    Square_Coordinates from_sc = square_to_coordinates(from);
    Square_Coordinates to_sc = square_to_coordinates(to);
    Square_Coordinates square_sc = square_to_coordinates(square);

    int row_diff = to_sc.row - from_sc.row;
    int col_diff = to_sc.col - from_sc.col;

    if (row_diff == 0 && col_diff == 0){ // If from and to are the same square
        return false;
    }

    if (row_diff == 0){ // If the ray is horizontal
        if (col_diff > 0){
            return square_sc.row == from_sc.row && square_sc.col >= from_sc.col && square_sc.col <= to_sc.col;
        } else {
            return square_sc.row == from_sc.row && square_sc.col <= from_sc.col && square_sc.col >= to_sc.col;
        }
    }

    if (col_diff == 0){ // If the ray is vertical
        if (row_diff > 0){
            return square_sc.col == from_sc.col && square_sc.row >= from_sc.row && square_sc.row <= to_sc.row;
        } else {
            return square_sc.col == from_sc.col && square_sc.row <= from_sc.row && square_sc.row >= to_sc.row;
        }
    }

    if (row_diff == col_diff){ // If the ray is diagonal (positive slope)
        if (row_diff > 0){
            return square_sc.row - from_sc.row == square_sc.col - from_sc.col && square_sc.row >= from_sc.row && square_sc.row <= to_sc.row;
        } else {
            return square_sc.row - from_sc.row == square_sc.col - from_sc.col && square_sc.row <= from_sc.row && square_sc.row >= to_sc.row;
        }
    }

    if (row_diff == -col_diff){ // If the ray is diagonal (negative slope)
        if (row_diff > 0){
            return square_sc.row - from_sc.row == from_sc.col - square_sc.col && square_sc.row >= from_sc.row && square_sc.row <= to_sc.row;
        } else {
            return square_sc.row - from_sc.row == from_sc.col - square_sc.col && square_sc.row <= from_sc.row && square_sc.row >= to_sc.row;
        }
    }

    return false;
}