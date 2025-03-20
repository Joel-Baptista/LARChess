#include "../../src/headers/game.h"

int main()
{
    std::vector<std::vector<std::string>> data_csv;
    std::vector<std::string> headers;
    bool hasHeaders = true;
    std::ifstream file("/home/jbaptista/projects/LARChess/AlphaZero/datasets/dataset.csv");
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

    for (int i = 0; i < data_csv.size(); i++)
    {

        auto row = data_csv[i];

        auto game = std::make_shared<Game>();
        
        std::string fen1 = row[0];
        
        game->m_Board->parse_fen(fen1.c_str());
        
        std::string fen2 = game->m_Board->get_fen();

        if (fen1 != fen2)
        {
            std::cout << "Found error in mapping in fen: " << fen1 << std::endl;
        }

    }


    return 0;
}