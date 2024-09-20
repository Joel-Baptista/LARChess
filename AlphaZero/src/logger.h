#pragma once
#include <iostream>

#include <fstream>
#include <string>
#include <ctime>

#include <filesystem>
#include <set>
#include <unordered_map>

class Logger {
public:
    Logger(bool debug);
    ~Logger();

    void logMessage(const std::string& message, const std::string& filename);
    void initLogFile(const std::string& filename);
    std::string initLogFiles(const std::string& path);

    void logTrain(std::string message);
    void logEval(std::string message);
    void logGrad(std::string message);
    void logConfig(std::unordered_map<std::string, std::string> config);

    void log(std::string message);

    bool debug;
    std::string model_path;
};