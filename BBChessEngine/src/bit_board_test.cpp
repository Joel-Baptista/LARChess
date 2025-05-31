#include <iostream>
#include <string>
#include "bit_board.h"

#include <fstream>
#include <sstream>
#include <vector>
#include <cassert>

int main() {
    BitBoard bit_board;

    

    // Open the CSV file
    std::ifstream file("../../AlphaZero/datasets/game_dataset.csv");
    if (!file.is_open()) {
        std::cerr << "Failed to open positions.csv" << std::endl;
        return 1;
    }

    std::cout << "File opened successfully!" << std::endl;

    std::string line;
    int total_tests = 0;
    int failed_tests = 0;

    // Ignore the first line (header)
    if (!std::getline(file, line)) {
        std::cerr << "Failed to read the header line from positions.csv" << std::endl;
        return 1;
    }

    while (std::getline(file, line)) {
        total_tests++;
        std::istringstream ss(line);
        std::string current_position_str, move_str, expected_next_position_str;

        // Read the current position, move, and expected next position from the CSV
        if (!std::getline(ss, current_position_str, ',') ||
            !std::getline(ss, move_str, ',') ||
            !std::getline(ss, expected_next_position_str, ',')) {
            std::cerr << "Malformed line in positions.csv: " << line << std::endl;
            failed_tests++;
            continue;
        }

        // Set the current position on the bitboard
        bit_board.parse_fen(current_position_str.c_str());

        // Apply the move
        bit_board.make_player_move(move_str.c_str());

        // Get the resulting position
        std::string predicted_next_position = bit_board.get_fen();

        // Compare the actual next position with the expected next position
        if (predicted_next_position != expected_next_position_str) {
            failed_tests++;
            std::cerr << "Test failed!" << std::endl;
            std::cerr << "Current position: " << current_position_str << std::endl;
            std::cerr << "Move: " << move_str << std::endl;
            std::cerr << "Expected next position: " << expected_next_position_str << std::endl;
            std::cerr << "Predicted next position: " << predicted_next_position << std::endl;
            std::cerr << "----------------------------------------" << std::endl;
        }
    }

    // Print the test results
    std::cout << "Total tests: " << total_tests << std::endl;
    std::cout << "Failed tests: " << failed_tests << std::endl;
    if (total_tests > 0) {
        double failure_percentage = (static_cast<double>(failed_tests) / total_tests) * 100.0;
        std::cout << "Failure percentage: " << failure_percentage << "%" << std::endl;
    }

    file.close();
    return 0;
}