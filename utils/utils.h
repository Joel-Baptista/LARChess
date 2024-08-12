#ifndef UTILS_H
#define UTILS_H
#include <string>

bool isupper(char c);
int find_nth(std::string s, char c, int n);
std::string coordinates_to_square(int row, int col);
struct Square_Coordinates { int row; int col;};
Square_Coordinates square_to_coordinates(std::string square);
bool is_in_ray(std::string from, std::string to, std::string square);
int positive_board(int x);

#endif // UTILS_H