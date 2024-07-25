#include "move.h"

Move::Move(const std::string& from, const std::string& to) : from(from), to(to) {
    this->promotion = ' '; // No promotion by default
}
Move::Move(const std::string& from, const std::string& to, const char& promotion) : from(from), to(to), promotion(promotion) {}

std::string Move::getFrom() const {
    return from;
}

std::string Move::getTo() const {
    return to;
}

char Move::getPromotion() const {
    return promotion;
}
