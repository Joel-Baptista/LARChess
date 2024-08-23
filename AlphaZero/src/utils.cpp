#include "utils.h"

// Function to get the current timestamp
std::string getCurrentTimestamp() {
    std::time_t now = std::time(nullptr);
    char buf[100];
    std::strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", std::localtime(&now));
    return buf;
}

// Function to log a message to a file
void logMessage(const std::string& message, const std::string& filename) {
    // Open the file in append mode
    std::ofstream logFile;
    logFile.open(filename, std::ios_base::app);
    std::cout << message << std::endl;
    // Check if the file opened successfully
    if (!logFile.is_open()) {
        std::cerr << "Failed to open log file: " << filename << std::endl;
        return;
    }

    // Write the timestamp and message to the log file
    logFile << "[" << getCurrentTimestamp() << "] " << message << std::endl;

    // Close the file
    logFile.close();
}

// Function to generate Dirichlet noise
torch::Tensor dirichlet_noise(double alpha, int batch_size) {
    std::random_device rd;
    std::mt19937 rng(rd());
    std::gamma_distribution<> gamma_dist(alpha, 1.0);
    
    torch::Tensor noise = torch::zeros({batch_size, 8, 8, 73});
    
    for (int i = 0; i < batch_size; ++i) {
        std::vector<double> sample(8 * 8 * 73);
        double sum = 0.0;
        for (int j = 0; j < 8 * 8 * 73; ++j) {
            sample[j] = gamma_dist(rng);
            sum += sample[j];
        }
        for (int j = 0; j < 73; ++j) {
            for (int k = 0; k < 8; ++k) 
            {
                for (int l = 0; l < 8; ++l) 
                {
                    int idx_sample = j * 8 * 8 + k * 8 + l;
                    noise[i][l][k][j] = sample[idx_sample] / sum;
                }
            }
        }
    }
    
    return noise;
}

void initLogFile(const std::string& filename) {
    // Check if the file exists
    std::ifstream file(filename);
    if (file) {
        // If it exists, delete the file
        file.close();
        if (std::remove(filename.c_str()) != 0) {
            std::cerr << "Error deleting log file: " << filename << std::endl;
        } else {
            std::cout << "Previous log file deleted: " << filename << std::endl;
        }
    }
}

void copy_weights(const torch::nn::Module& source, torch::nn::Module& target) {
    auto source_params = source.parameters();
    auto target_params = target.parameters();
    
    // Ensure the number of parameters match
    if (source_params.size() != target_params.size()) {
        throw std::runtime_error("Source and target networks have different numbers of parameters.");
    }
    
    // Copy parameters
    for (size_t i = 0; i < source_params.size(); ++i) {
        target_params[i].data().copy_(source_params[i]);
    }
    std::cout << "Weights copied successfully." << std::endl;
}

