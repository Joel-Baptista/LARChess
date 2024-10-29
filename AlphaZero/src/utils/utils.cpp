#include "utils.h"

namespace fs = std::filesystem;

// Function to get the current timestamp
std::string getCurrentTimestamp() {
    std::time_t now = std::time(nullptr);
    char buf[100];
    std::strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", std::localtime(&now));
    return buf;
}

// Function to generate Dirichlet noise
// void dirichlet_noise(torch::Tensor& noise, std::vector<float> alpha_vector, int& batch_size) {
//     std::random_device rd;
//     std::mt19937 rng(rd());
//     std::gamma_distribution<> gamma_dist(alpha, 1.0);
//     
//     for (int i = 0; i < batch_size; ++i) {
//         std::vector<double> sample(8 * 8 * 73);
//         double sum = 0.0;
//         for (int j = 0; j < 8 * 8 * 73; ++j) {
//             sample[j] = gamma_dist(rng);
//             sum += sample[j];
//         }
//         for (int j = 0; j < 73; ++j) {
//             for (int k = 0; k < 8; ++k) 
//             {
//                 for (int l = 0; l < 8; ++l) 
//                 {
//                     int idx_sample = j * 8 * 8 + k * 8 + l;
//                     noise[i][l][k][j] = sample[idx_sample] / sum;
//                 }
//             }
//         }
//     }
// }

void add_dirichlet_noise(torch::Tensor& action, const std::vector<double>& alpha, const float& epsilon) {
    size_t K = alpha.size();
    std::vector<double> samples(K);
    std::random_device rd;
    std::mt19937 gen(rd());
    std::gamma_distribution<double> gammaDist;

    double sumSamples = 0.0;

    // Generate K samples from Gamma distribution with shape alpha_i
    for (size_t i = 0; i < K; ++i) {
        gammaDist = std::gamma_distribution<double>(alpha[i], 1.0); // scale parameter = 1
        samples[i] = gammaDist(gen);
        sumSamples += samples[i];
    }

    // Normalize the samples to lie on the simplex (i.e., they sum to 1)
    for (size_t i = 0; i < K; ++i) {
        samples[i] /= sumSamples;
    }

    action = action.view({-1});

    // Add the Dirichlet noise to the action probabilities
    int alpha_count = 0;
    for (size_t j = 0; j < action.size(0); ++j) {
        if (action[j].item<float>() > 0) {
            action[j] = (1 - epsilon) * action[j].item<float>() + epsilon * samples[alpha_count];
            alpha_count++;
        }
    }
    action = action.view({8, 8, 73});
}

// Function to log a message to a file
void logMessage(const std::string& message, const std::string& filename) {
    // Open the file in append mode
    std::ofstream logFile;
    logFile.open(filename, std::ios_base::app);

    // Check if the file opened successfully
    if (!logFile.is_open()) {
        std::cerr << "Failed to open log file: " << filename << std::endl;
        return;
    }

    // Write the timestamp and message to the log file
    logFile << message << std::endl;

    // Close the file
    logFile.close();
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

std::string initLogFiles(const std::string& path) {
    std::set<int> existing_numbers;

    // Iterate through the models folder
    for (const auto& entry : fs::directory_iterator(path)) {
        if (entry.is_directory()) {
            std::string folder_name = entry.path().filename().string();
            if (folder_name.find("model") == 0) {
                std::string number_str = folder_name.substr(5); // Extract the number part
                try {
                    int number = std::stoi(number_str);
                    existing_numbers.insert(number);
                } catch (const std::invalid_argument& e) {
                    // Handle case where the folder name isn't in the expected format
                    std::cerr << "Invalid folder name format: " << folder_name << std::endl;
                }
            }
        }
    }

    // Find the smallest unused number
    int smallest_unused_number = 1;
    while (existing_numbers.count(smallest_unused_number)) {
        ++smallest_unused_number;
    }

    // Create the new folder
    std::string new_folder_name = path + "/model" + std::to_string(smallest_unused_number);
    fs::create_directory(new_folder_name);

    std::cout << "Created folder: " << new_folder_name << std::endl;

    // // Create the log file
    initLogFile(new_folder_name + "/train.csv");
    initLogFile(new_folder_name + "/eval.csv");
    logMessage("iter,win,loss,draw", new_folder_name + "/eval.csv");
    initLogFile(new_folder_name + "/log.txt");
    initLogFile(new_folder_name + "/config.json");

    return new_folder_name;
}

void copy_weights(const torch::nn::Module& source, torch::nn::Module& target) {
    
    torch::NoGradGuard no_grad;
    auto source_params = source.parameters();
    auto target_params = target.parameters();
    
    // Ensure the number of parameters match
    if (source_params.size() != target_params.size()) {
        throw std::runtime_error("Source and target networks have different numbers of parameters.");
    }

    // Copy parameters
    for (size_t i = 0; i < source_params.size(); ++i) {
        target_params[i].copy_(source_params[i].clone().detach());
    }

    auto target_buffers = target.buffers();
    auto source_buffers = source.buffers();

    for (size_t i = 0; i < source_buffers.size(); ++i) {
        target_buffers[i].copy_(source_buffers[i].detach());
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

double calculate_gradient_norm(const std::vector<torch::Tensor>& parameters) {
    double total_norm = 0.0;
    for (const auto& param : parameters) {
        if (param.grad().defined()) {
            auto grad_norm = param.grad().norm().item<double>();
            total_norm += grad_norm * grad_norm;
        }
    }
    return std::sqrt(total_norm);
}

float get_prob_uni()
{
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<> dis(0.0, 1.0);
    return dis(gen);
}

void load_Resnet(std::string path, std::shared_ptr<ResNetChess> m_ResNetChess, std::shared_ptr<torch::Device> m_Device)
{
    std::vector<int64_t> shape = {1, 19, 8, 8};
    torch::Tensor encoded_state = torch::rand(shape, torch::kFloat32).to(*m_Device); // Initialize the tensor with zeros
    torch::load(m_ResNetChess, path + "model.pt", *m_Device);
    m_ResNetChess->eval();

    auto output1= m_ResNetChess->forward(encoded_state);
    
    m_ResNetChess->to(*m_Device, torch::kFloat32);

    // clamp_small_weights(*m_ResNetChess, 1e-15);

    auto output2 = m_ResNetChess->forward(encoded_state);

    if (torch::allclose(output1.policy, output2.policy) || torch::allclose(output1.value, output2.value))
    {
        std::cout << "Model loaded successfully" << std::endl;
    }
    else
    {
        std::cout << "Be carefull! Clamping the weights significantly altered the network" << std::endl;
    }

}

std::vector<ChessPosition> readChessCSV(const std::string& filename) {
    std::vector<ChessPosition> positions;
    std::ifstream file(filename);
    std::string line, token;

    // Read header and ignore
    std::getline(file, line);

    // Read each line of the CSV
    while (std::getline(file, line)) {
        std::stringstream ss(line);
        ChessPosition pos;

        // Read epd
        std::getline(ss, pos.epd, ',');
        // Read move
        std::getline(ss, pos.move, ',');
        // Read eval (convert to int)
        std::getline(ss, token, ',');
        pos.eval = std::stoi(token);
        // Read depth (convert to int)
        std::getline(ss, token, ',');
        pos.depth = std::stoi(token);
        // Read pv (principal variation)
        std::getline(ss, pos.pv, ',');
        // Read engine
        std::getline(ss, pos.engine, ',');

        positions.push_back(pos);
    }

    return positions;
}

std::vector<PuzzleData> readPuzzleCSV(const std::string& filename) {
    std::vector<PuzzleData> puzzles;
    std::ifstream file(filename);
    std::string line, token;

    // Read header and ignore
    std::getline(file, line);

    // Read each line of the CSV
    while (std::getline(file, line)) {
        std::stringstream ss(line);
        PuzzleData puzzle;

        // Read PuzzleId
        std::getline(ss, puzzle.PuzzleId, ',');
        // Read FEN
        std::getline(ss, puzzle.epd, ',');
        // Read Moves
        std::getline(ss, puzzle.move, ',');
        // Read Rating (convert to int)
        std::getline(ss, token, ',');
        puzzle.Rating = std::stoi(token);
        // Read RatingDeviation (convert to int)
        std::getline(ss, token, ',');
        puzzle.RatingDeviation = std::stoi(token);
        // Read Popularity (convert to int)
        std::getline(ss, token, ',');
        puzzle.Popularity = std::stoi(token);
        // Read NbPlays (convert to int)
        std::getline(ss, token, ',');
        puzzle.NbPlays = std::stoi(token);
        // Read Themes
        std::getline(ss, puzzle.Themes, ',');
        // Read GameUrl
        std::getline(ss, puzzle.GameUrl, ',');
        // Read OpeningTags
        std::getline(ss, puzzle.OpeningTags, ',');

        puzzles.push_back(puzzle);
    }

    return puzzles;
}
