#include "../../src/headers/game.h"

int main()
{
    std::vector<std::vector<std::string>> data_csv;
    std::vector<std::string> headers;
    bool hasHeaders = true;
    std::ifstream file("/home/joel/projects/LARChess/AlphaZero/datasets/processed_tactic_evals.csv");
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
        // std::cout << "Row: " << i << std::endl;
        // std::cout << "Fen: " << data_csv[i][0] << std::endl;
        // std::cout << "Move: " << data_csv[i][1] << std::endl;
        if (i % (data_csv.size() / 100) == 0)
        {
            std::cout << "Progress: " << (i * 100) / data_csv.size() << "%" << std::endl;
        }
        std::cout << "Row: " << i << std::endl;
        auto row = data_csv[i];

        auto game = std::make_shared<Game>();
        
        std::string fen1 = row[0];
        std::string move = row[1];
        
        game->m_Board->parse_fen(fen1.c_str());
        
        std::string fen2 = game->m_Board->get_fen();

        torch::Tensor encoded_state = torch::zeros({1, 22, 8, 8});
        state game_state = game->get_state();

        auto start = std::chrono::high_resolution_clock::now();
        get_encoded_state(encoded_state, game_state);
        auto end = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> elapsed = end - start;
        std::cout << "Time taken for get_encoded_state: " << elapsed.count() << " seconds" << std::endl;
        
        state current_state;
        memset(current_state.bitboards, 0ULL, sizeof(current_state.bitboards));

        auto start_decoded_state = std::chrono::high_resolution_clock::now();
        get_decoded_state(current_state, encoded_state);
        auto end_decoded_state = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> elapsed_decoded_state = end_decoded_state - start_decoded_state;
        std::cout << "Time taken for get_decoded_state: " << elapsed_decoded_state.count() << " seconds" << std::endl;

        auto start_encoded_action = std::chrono::high_resolution_clock::now();
        torch::Tensor encoded_action = game->get_encoded_action(move, game_state.side);
        auto end_encoded_action = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> elapsed_encoded_action = end_encoded_action - start_encoded_action;
        std::cout << "Time taken for get_encoded_action: " << elapsed_encoded_action.count() << " seconds" << std::endl;

        auto start_decoded_action = std::chrono::high_resolution_clock::now();
        std::string decoded_move = game->decode_action(game_state, encoded_action);
        auto end_decoded_action = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> elapsed_decoded_action = end_decoded_action - start_decoded_action;
        std::cout << "Time taken for decode_action: " << elapsed_decoded_action.count() << " seconds" << std::endl;

        std::cout << "-------------------------------------------" << std::endl;

        if (move != decoded_move)
        {
            std::cout << "Row: " << i << std::endl;
            std::cout << "Found error in mapping in move: " << fen1 << std::endl;
            std::cout << "Expected move: " << move << ", Got: " << decoded_move << std::endl;
            break;
        }

        if (current_state.side != game_state.side)
        {
            std::cout << "Found error in mapping in side: " << fen1 << std::endl;
            std::cout << "Expected side: " << game_state.side << ", Got: " << current_state.side << std::endl;
            break;
        }
        if (current_state.castle_rights != game_state.castle_rights)
        {
            std::cout << "Found error in mapping in castle_rights: " << fen1 << std::endl;
            std::cout << "Expected castle_rights: " << game_state.castle_rights << ", Got: " << current_state.castle_rights << std::endl;
            break;
        }
        if (current_state.halfmove != game_state.halfmove)
        {
            std::cout << "Found error in mapping in halfmove: " << fen1 << std::endl;
            std::cout << "Expected halfmove: " << game_state.halfmove << ", Got: " << current_state.halfmove << std::endl;
            break;
        }
        if (current_state.en_passant_square != game_state.en_passant_square)
        {
            std::cout << "Found error in mapping in en_passant_square: " << fen1 << std::endl;
            std::cout << "Expected en_passant_square: " << game_state.en_passant_square << ", Got: " << current_state.en_passant_square << std::endl;
            break;
        }
        if (memcmp(current_state.bitboards, game_state.bitboards, sizeof(U64) * 12) != 0)
        {
            std::cout << "Expected bitboards: " << game_state.bitboards << ", Got: " << current_state.bitboards << std::endl;
            break;
        }

        if (fen1 != fen2)
        {
            std::cout << "Found error in mapping in fen: " << fen1 << std::endl;
            std::cout << "Expected fen: " << fen1 << ", Got: " << fen2 << std::endl;
            break;
        }
    }


    return 0;
}