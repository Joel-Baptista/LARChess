#ifndef UTILS_H
#define UTILS_H
#include <string>

struct Square_Coordinates { int row; int col;};
bool isupper(char c);
int find_nth(std::string s, char c, int n);
std::string coordinates_to_square(int row, int col);
Square_Coordinates square_to_coordinates(std::string square);

#endif // UTILS_H