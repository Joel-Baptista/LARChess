#include "../../src/headers/game.h"

int main()
{
    std::vector<std::vector<std::string>> data_csv;
    std::vector<std::string> headers;
    bool hasHeaders = true;
    std::ifstream file("/home/joel/projects/LARChess/AlphaZero/datasets/dataset.csv");
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

        torch::Tensor encoded_state = torch::zeros({1, 19, 8, 8});
        state game_state = game->get_state();
        get_encoded_state(encoded_state, game_state);

        state current_state;
        get_decoded_state(current_state, encoded_state);
        

        if (current_state.side != game_state.side)
        {
            std::cout << "Found error in mapping in side: " << fen1 << std::endl;
        }
        if (current_state.castle_rights != game_state.castle_rights)
        {
            std::cout << "Found error in mapping in castle_rights: " << fen1 << std::endl;
        }
        if (current_state.halfmove != game_state.halfmove)
        {
            std::cout << "Found error in mapping in halfmove: " << fen1 << std::endl;
        }
        if (current_state.en_passant_square != game_state.en_passant_square)
        {
            std::cout << "Found error in mapping in en_passant_square: " << fen1 << std::endl;
        }
        if (current_state.bitboards != game_state.bitboards)
        {
            std::cout << "Found error in mapping in bitboards: " << fen1 << std::endl;
        }


        if (fen1 != fen2)
        {
            std::cout << "Found error in mapping in fen: " << fen1 << std::endl;
        }
        else
        {
            std::cout << "Same Fen" << std::endl;
        }

    }


    return 0;
}