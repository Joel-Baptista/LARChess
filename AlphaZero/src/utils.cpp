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
void dirichlet_noise(torch::Tensor& noise,float& alpha, int& batch_size) {
    std::random_device rd;
    std::mt19937 rng(rd());
    std::gamma_distribution<> gamma_dist(alpha, 1.0);
    
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
}

void clamp_small_weights(torch::nn::Module& model, float threshold = 1e-6) {
    // Iterate over all parameters in the model

    for (auto& param : model.parameters()) {
        // Create a mask where the absolute value is smaller than the threshold
        auto mask = torch::abs(param.data()) < threshold;
        
        // Get the sign of the weights
        auto sign = torch::sign(param.data());
        
        // Create a tensor with the threshold value, matching the shape of the original tensor
        auto threshold_tensor = torch::full(param.data().sizes(), threshold, param.data().options());
        
        // Apply clamping manually
        param.data().copy_(torch::where(mask, sign * threshold_tensor, param.data()));
    }
}

