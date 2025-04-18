#include "../headers/dataset.h"

Dataset::Dataset(const std::string &filename, float train_split, c10::ScalarType precision, bool hasHeaders) : hasHeaders(hasHeaders), train_split(train_split), precision(precision)
{
    loadCSV(filename);
}

// Load CSV file into the dataset
void Dataset::loadCSV(const std::string &filename)
{
    std::ifstream file(filename);
    if (!file.is_open())
    {
        throw std::runtime_error("Could not open file");
    }

    std::string line;
    bool firstLine = true;

    while (std::getline(file, line))
    {
        std::stringstream ss(line);
        std::string value;
        std::vector<std::string> row;

        while (std::getline(ss, value, ','))
        {
            row.push_back(value);
        }

        if (firstLine && hasHeaders)
        {
            headers = row;
            firstLine = false;
        }
        else
        {
            data_csv.push_back(row);
        }
    }

    file.close();

    shuffle_csv();

    std::cout << "CSV loaded" << std::endl;

    Game game;
    data_train = std::make_shared<std::vector<data_point>>();
    data_eval = std::make_shared<std::vector<data_point>>();

    int split = (int)(train_split * data_csv.size());

    for (int i = 0; i < split; i++)
    {

        if (((i + 1) % (split / 10)) == 0)
        {
            std::cout << "Loading train data: " << (int)ceil((100 * ((float)(i) / split))) << "%" << std::endl;
        }

        auto row = data_csv[i];

        data_point dp;

        // std::cout << row[0] << std::endl;
        // std::cout << row[1] << std::endl;
        // std::cout << row[2] << std::endl;
        // std::cout << row[3] << std::endl;

        game.m_Board->parse_fen(row[0].c_str());

        auto current_state = game.get_state();

        torch::Tensor current_state_tensor = torch::zeros({1, 19, 8, 8}, precision); // Initialize the tensor with zeros

        game.get_encoded_state(current_state_tensor, current_state);
        dp.state = current_state_tensor;
        dp.action = game.get_encoded_action(row[1], current_state.side);
        dp.action.to(precision);

        float value = std::stof(row[2]);

        dp.value = torch::tensor(value, precision);
        data_train->push_back(dp);
    }

    auto data_size = data_csv.size();
    for (int i = split; i < data_size; i++)
    {

        if (((i + 1) % ((data_size - split) / 10)) == 0)
        {
            std::cout << "Loading evaluation data: " << (int)ceil((100 * ((float)(i - split) / (data_size - split)))) << "%" << std::endl;
        }

        auto row = data_csv[i];

        data_point dp;

        game.m_Board->parse_fen(row[0].c_str());

        auto current_state = game.get_state();

        torch::Tensor current_state_tensor = torch::zeros({1, 19, 8, 8}, precision); // Initialize the tensor with zeros

        game.get_encoded_state(current_state_tensor, current_state);
        dp.state = current_state_tensor;
        dp.action = game.get_encoded_action(row[1], current_state.side);
        dp.action.to(precision);

        float value = std::stof(row[2]);

        dp.value = torch::tensor(value, precision);
        data_eval->push_back(dp);
    }

    std::cout << "Loaded tensors" << std::endl;
}

void Dataset::shuffle_csv()
{
    std::random_device rd;                             // Obtain a random number from hardware
    std::mt19937 g(rd());                              // Seed the generator
    std::shuffle(data_csv.begin(), data_csv.end(), g); // Shuffle the rows
}

void Dataset::shuffle()
{
    std::random_device rd;                                   // Obtain a random number from hardware
    std::mt19937 g(rd());                                    // Seed the generator
    std::shuffle(data_train->begin(), data_train->end(), g); // Shuffle the rows
}