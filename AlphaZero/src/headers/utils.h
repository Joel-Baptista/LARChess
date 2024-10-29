#pragma once

#include <fstream>
#include <string>
#include <ctime>
#include <iostream>
#include <random>
#include <torch/torch.h>

#include "../include/ResNet.h"

#include <filesystem>
#include <set>

#define U64 unsigned long long

std::string getCurrentTimestamp();
void logMessage(const std::string& message, const std::string& filename);
void dirichlet_noise(torch::Tensor& noise, float& alpha, int& batch_size, int action_size);
void add_dirichlet_noise(torch::Tensor& action, const std::vector<double>& alpha, const float& epsilon);
void initLogFile(const std::string& filename);
std::string initLogFiles(const std::string& path);

void copy_weights(const torch::nn::Module& source, torch::nn::Module& target);
double calculate_gradient_norm(const std::vector<torch::Tensor>& parameters);

// Define a wrapper class for the array
class BitboardKey {
public:
    U64 bitboards[12];

    // Constructor
    BitboardKey(const U64 bbs[12]) {
        std::memcpy(bitboards, bbs, sizeof(bitboards));
    }

    // Equality operator
    bool operator==(const BitboardKey& other) const {
        return std::memcmp(bitboards, other.bitboards, sizeof(bitboards)) == 0;
    }
};

// Define the hash function for the wrapper class
struct BitboardHash {
    std::size_t operator()(const BitboardKey& key) const {
        std::size_t hash = 0;
        for (const auto& value : key.bitboards) {
            hash ^= std::hash<U64>{}(value) + 0x9e3779b9 + (hash << 6) + (hash >> 2);
        }
        return hash;
    }
};

void clamp_small_weights(torch::nn::Module& model, float threshold);

float get_prob_uni();

void load_Resnet(std::string path, std::shared_ptr<ResNetChess> m_ResNetChess, std::shared_ptr<torch::Device> m_Device);

struct ChessPosition {
    std::string epd;
    std::string move;
    int eval;
    int depth;
    std::string pv;
    std::string engine;
};

std::vector<ChessPosition> readChessCSV(const std::string& filename);

struct PuzzleData {
    std::string PuzzleId;
    std::string epd;
    std::string move;
    int Rating;
    int RatingDeviation;
    int Popularity;
    int NbPlays;
    std::string Themes;
    std::string GameUrl;
    std::string OpeningTags;
};

std::vector<PuzzleData> readPuzzleCSV(const std::string& filename);