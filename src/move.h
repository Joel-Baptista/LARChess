#ifndef MOVE_H
#define MOVE_H

#include <string>
#include <array>
#include <vector>

class Move {
public:
    Move(const std::string& from, const std::string& to);
    Move(const std::string& from, const std::string& to, const char& promotion);
    std::string getFrom() const;
    std::string getTo() const;
    char getPromotion() const;
private:
    std::string from;
    std::string to;
    char promotion;
};




#endif // MOVE_H
