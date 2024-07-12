#ifndef MOVE_H
#define MOVE_H

#include <string>

class Move {
public:
    Move(const std::string& from, const std::string& to);
    std::string getFrom() const;
    std::string getTo() const;

private:
    std::string from;
    std::string to;
};

#endif // MOVE_H
