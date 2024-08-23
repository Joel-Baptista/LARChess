#pragma once

#include "game.h"

#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <stdexcept>
#include <algorithm>
#include <numeric>

struct 
{
    torch::Tensor state;
    torch::Tensor action;
    torch::Tensor value;
} typedef data_point;


class Dataset {
private:

    std::shared_ptr<std::vector<data_point>> data_train;
    std::shared_ptr<std::vector<data_point>> data_eval;
    std::vector<std::vector<std::string>> data_csv;
    std::vector<std::string> headers;
    bool hasHeaders;
    float train_split;

public:
    // Constructor to load the CSV file
    Dataset(const std::string& filename, float train_split ,bool hasHeaders = true);

    // Load CSV file into the dataset
    void loadCSV(const std::string& filename);

    // Get the number of rows in the dataset
    size_t train_size() const {
        return data_train->size();
    }

    size_t eval_size() const {
        return data_eval->size();
    }

    // Get the number of columns in the dataset
    size_t colCount() const {
        return data_csv.empty() ? 0 : data_csv[0].size();
    }

    // Get the header names
    const std::vector<std::string>& getHeaders() const {
        return headers;
    }

    // Access a row by index
    const data_point& getTrain(size_t index) const {
        if (index >= train_size()) {
            throw std::out_of_range("Row index out of range");
        }
        return data_train->at(index);
    }

    const data_point& getEval(size_t index) const {
        if (index >= eval_size()) {
            throw std::out_of_range("Row index out of range");
        }
        return data_eval->at(index);
    }

    // Access a column by index or name
    // std::vector<std::string> getColumn(size_t index) const {
    //     if (index >= colCount()) {
    //         throw std::out_of_range("Column index out of range");
    //     }
    //     std::vector<std::string> column;
    //     for (const auto& row : data_csv) {
    //         column.push_back(row[index]);
    //     }
    //     return column;
    // }

    // std::vector<std::string> getColumn(const std::string& header) const {
    //     auto it = std::find(headers.begin(), headers.end(), header);
    //     if (it == headers.end()) {
    //         throw std::invalid_argument("Header not found");
    //     }
    //     size_t index = std::distance(headers.begin(), it);
    //     return getColumn(index);
    // }

    // // Get a specific data point by row and column index
    // const std::string& getDataPoint(size_t rowIndex, size_t colIndex) const {
    //     return getRow(rowIndex).at(colIndex);
    // }

    // Get basic statistics for a numerical column (mean, min, max)
    // double getMean(const std::string& columnName) const {
    //     std::vector<std::string> column = getColumn(columnName);
    //     std::vector<double> numericalValues;

    //     for (const auto& value : column) {
    //         numericalValues.push_back(std::stod(value));
    //     }

    //     double sum = std::accumulate(numericalValues.begin(), numericalValues.end(), 0.0);
    //     return sum / numericalValues.size();
    // }

    // double getMin(const std::string& columnName) const {
    //     std::vector<std::string> column = getColumn(columnName);
    //     std::vector<double> numericalValues;

    //     for (const auto& value : column) {
    //         numericalValues.push_back(std::stod(value));
    //     }

    //     return *std::min_element(numericalValues.begin(), numericalValues.end());
    // }

    // double getMax(const std::string& columnName) const {
    //     std::vector<std::string> column = getColumn(columnName);
    //     std::vector<double> numericalValues;

    //     for (const auto& value : column) {
    //         numericalValues.push_back(std::stod(value));
    //     }

    //     return *std::max_element(numericalValues.begin(), numericalValues.end());
    // }

    void shuffle_csv();
    void shuffle();
};