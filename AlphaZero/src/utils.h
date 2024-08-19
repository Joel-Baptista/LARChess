#pragma once

#include <fstream>
#include <string>
#include <ctime>
#include <iostream>
#include <random>

#include "game.h"

std::string getCurrentTimestamp();
void logMessage(const std::string& message, const std::string& filename);
torch::Tensor dirichlet_noise(double alpha, int batch_size);