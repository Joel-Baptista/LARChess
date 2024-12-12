#include "../../src/headers/game.h"
#include "../../src/headers/mcts.h"

#include "../../../BBChessEngine/src/bit_board.h"
#include "../../src/include/json.hpp"
#include <torch/script.h> // For TorchScript


using json = nlohmann::json; 

int main()
{
    std::ifstream config_file("../cfg/config.json");
    if (!config_file.is_open()) {
        std::cerr << "Could not open the config file!" << std::endl;
        return 1;
    }

    auto gen = at::detail::createCPUGenerator(42);

    // Parse the JSON data
    json config;
    try {
        config_file >> config;
    } catch (const json::parse_error& e) {
        std::cerr << "JSON parse error: " << e.what() << std::endl;
        return 1;
    }

    std::string initial_position = config.value("initial_position", initial_position);
    std::string human_player = config.value("human_player", "w");
    int num_searches = config.value("num_searches", 5);
    float dichirlet_alpha = config.value("dichirlet_alpha", 0.5);
    float dichirlet_epsilon = config.value("dichirlet_epsilon", 0.3);
    float C = config.value("C", 1.41);
    float temperature = config.value("temperature", 1.0);
    int num_resblocks = config.value("num_resblocks", 7);
    int num_channels = config.value("num_channels", 256);
    std::string model_path = config.value("model_path", "");
    std::string device = config.value("device", "cpu");

    auto model_device = std::make_unique<torch::Device>(torch::kCPU);

    if (torch::cuda::is_available() && (device.find("cuda") != device.npos))
    {
        auto dots = device.find(":");
        int device_id = 0;
        if (dots != device.npos)
        {
            device_id = std::stoi(device.substr(dots + 1, device.npos - dots));
        }

        model_device = std::make_unique<torch::Device>(torch::kCUDA, device_id);

    }
    else
    {
        model_device = std::make_unique<torch::Device>(torch::kCPU);
    }

    auto module = torch::jit::load("resnet_chess.pt");

    auto model = std::make_shared<ResNetChess>(num_resblocks, num_channels, 0.0, *model_device);

    auto mcts = std::make_unique<MCTS>(model, num_searches, dichirlet_alpha, dichirlet_epsilon, C);

    if (model_path != "")
        torch::load(model, model_path + "model.pt", *model_device);

    std::vector<std::shared_ptr<Game>> games;
    std::vector<SPG*> spGames;

    games.push_back(std::make_shared<Game>());
    games.at(0)->m_Board->parse_fen(start_position);
    SPG* spg = new SPG(games.at(0));
    spGames.push_back(spg);
    auto results = mcts->predict(&spGames);

    auto st = get_time_ms();

    results = mcts->predict(&spGames);

    std::cout << "Inference time: " << ((get_time_ms() - st) / 1000.0f) << std::endl;

    torch::Tensor action_probs = std::get<0>(results.at(0));
    float state_value = std::get<1>(results.at(0));

    std::vector<int64_t> shape = {8, 8, 73}; 
    torch::Tensor valid_moves = torch::zeros(shape, torch::kFloat32);

    moves move_list;
    spGames.at(0)->game->m_Board->get_alpha_moves(&move_list);

    get_valid_moves_encoded(valid_moves, spGames.at(0)->pCurrentNode->node_state, move_list);

    auto decoded_actions = games.at(0)->decode_actions(spGames.at(0)->current_state, action_probs, valid_moves);
        
    std::cout << "Bot move: " << decoded_actions.at(0).action << std::endl;
    std::cout << "Move Probability: " << decoded_actions.at(0).probability << std::endl;

    return 0;
}