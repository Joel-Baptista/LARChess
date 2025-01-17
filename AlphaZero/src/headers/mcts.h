#pragma once

#include "../../../BBChessEngine/src/bit_board.h"
#include <vector>
#include "game.h"
#include <array>
#include <memory>
#include <unordered_map>

#define  matrix4D(b, c, h, w, type) std::array<std::array<std::array<std::array<type, w>, h>, c>, b>


class Node
{
    public:
        Node(std::shared_ptr<Game> game, Node* parent, std::string action, float C, float prior, int visit_count);
        ~Node();

        bool is_fully_expanded();
        Node* select();
        float get_ubc(Node* child);
        void expand(torch::Tensor action_probs, torch::Tensor valid_moves);
        void backpropagate(float value);

        // Pass this variables to private once finished debugging
        std::vector<Node*> pChildren; 
        std::string action;
        int visit_count; 
        state node_state;
        float value_sum;

    private:

        std::shared_ptr<Game> game;
        Node* parent;
        float C;
        float prior;

};

class SPG
{
    public:
        SPG(std::shared_ptr<Game> board);
        ~SPG();

        state initial_state;
        state current_state;

        bool early_stop;

        std::vector<memory_item> memory;

        Node* pRoot;
        Node* pCurrentNode;
        std::shared_ptr<Game> game;
        std::unordered_map<BitboardKey, int, BitboardHash> repeated_states;

        void reset();

    private:


};

class MCTS
{
    public:
        MCTS(std::shared_ptr<ResNetChess> model, int num_searches, float dichirlet_alpha, float dichirlet_epsilon, float C);
        MCTS(std::shared_ptr<ResNetChess> model, int thread_id, int num_searches, float dichirlet_alpha, float dichirlet_epsilon, float C);
        ~MCTS();


        int get_num_searches() { return num_searches; }

        void search(std::vector<SPG*>* spGames, bool deterministic);
        void search(std::vector<SPG*>* spGames) {search(spGames, false);};
        std::vector<std::tuple<torch::Tensor, float>> predict(std::vector<SPG*>* spGames, bool deterministic);
        std::vector<std::tuple<torch::Tensor, float>> predict(std::vector<SPG*>* spGames) { return predict(spGames, false);};

        // void search(std::vector<SPG*>* spGames) { std::vector<c10::cuda::CUDAStream> cuda_streams; search(spGames, cuda_streams); }

        void set_dichirlet_epsilon(float epsilon) { dichirlet_epsilon = epsilon; }
        void set_C(float c) { C = c; }
        void set_num_searches(int s) { num_searches = s; }
        std::unordered_map<BitboardKey, int, BitboardHash> get_boards_visited() { return boards_visited; }

    private:
        int num_searches;
        float dichirlet_alpha;
        float dichirlet_epsilon;
        int search_depth;

        std::shared_ptr<ResNetChess> m_model;
        std::unordered_map<BitboardKey, int, BitboardHash> boards_visited;

        float C;
        int thread_id;
};


