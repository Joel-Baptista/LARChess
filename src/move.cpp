#include "move.h"

Move::Move(const std::string& from, const std::string& to) : from(from), to(to) {}

std::string Move::getFrom() const {
    return from;
}

std::string Move::getTo() const {
    return to;
}
