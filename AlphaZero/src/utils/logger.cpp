#include "logger.h"
#include "utils.h"

namespace fs = std::filesystem;

Logger::Logger(bool debug) 
{
    this->debug = debug; 
    this->model_path = ""; 
}
Logger::~Logger() 
{

}
void Logger::logMessage(const std::string& message, const std::string& filename)
{
    // Open the file in append mode
    
    if (debug) {
        std::cout << message << std::endl;
        return;
    }
    
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
void Logger::initLogFile(const std::string& filename)
{
    std::ifstream file(filename);
    if (file && !debug) {
        // If it exists, delete the file
        file.close();
        if (std::remove(filename.c_str()) != 0) {
            std::cerr << "Error deleting log file: " << filename << std::endl;
        } else {
            std::cout << "Previous log file deleted: " << filename << std::endl;
        }
    }
}
std::string Logger::initLogFiles(const std::string& path)
{
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
    
    if (!debug) fs::create_directory(new_folder_name);

    std::cout << "Created folder: " << new_folder_name << std::endl;

    // // Create the log file
    if (!debug)
    {
        initLogFile(new_folder_name + "/loss.csv");
        logMessage("iter,loss", new_folder_name + "/loss.csv");
        initLogFile(new_folder_name + "/val_loss.csv");
        logMessage("iter,loss", new_folder_name + "/val_loss.csv");
        initLogFile(new_folder_name + "/pi_loss.csv");
        logMessage("iter,loss", new_folder_name + "/pi_loss.csv");
        initLogFile(new_folder_name + "/eval.csv");
        logMessage("iter,win,loss,draw", new_folder_name + "/eval.csv");
        initLogFile(new_folder_name + "/grads.csv");
        logMessage("iter,grad", new_folder_name + "/grads.csv");
        initLogFile(new_folder_name + "/lr.csv");
        logMessage("iter,lr", new_folder_name + "/lr.csv");
        initLogFile(new_folder_name + "/ep_len.csv");
        logMessage("iter,len", new_folder_name + "/ep_len.csv");
        initLogFile(new_folder_name + "/ep_res.csv");
        logMessage("iter,len", new_folder_name + "/ep_res.csv");
        initLogFile(new_folder_name + "/fps.csv");
        logMessage("iter,fps", new_folder_name + "/fps.csv");
        initLogFile(new_folder_name + "/n_games.csv");
        logMessage("iter,n_games", new_folder_name + "/n_games.csv");
        initLogFile(new_folder_name + "/env_steps.csv");
        logMessage("iter,n_games", new_folder_name + "/env_steps.csv");
        initLogFile(new_folder_name + "/log.txt");
        initLogFile(new_folder_name + "/config.json");
    }

    this->model_path = new_folder_name;

    return new_folder_name;
}
void Logger::logTrain(std::string message)
{
    if (debug) return;
    logMessage(message, model_path + "/loss.csv");
}
void Logger::logEval(std::string message)
{
    if (debug) return;
    logMessage(message, model_path + "/eval.csv");
}

void Logger::logGrad(std::string message)
{
    if (debug) return;
    logMessage(message, model_path + "/grads.csv");
}

void Logger::log(std::string message)
{
    std::cout << "[" << getCurrentTimestamp() << "] " << message << std::endl;
    
    if (debug) return;
    
    logMessage( "[" + getCurrentTimestamp() + "] " + message, model_path + "/log.txt");
}

void Logger::logConfig(std::unordered_map<std::string, std::string> config)
{
    if (debug) return;
    logMessage("{", model_path + "/config.json");
    for (auto const& [key, val] : config) {
        logMessage("    \"" + key + "\": \"" + val + "\",", model_path + "/config.json");
    }
    logMessage("}", model_path + "/config.json");
}
