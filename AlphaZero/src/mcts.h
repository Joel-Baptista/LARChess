#pragma once

#include "../../BBChessEngine/src/bit_board.h"
#include <vector>
#include "game.h"
#include <array>
#include <memory>

#define  matrix4D(b, c, h, w, type) std::array<std::array<std::array<std::array<type, w>, h>, c>, b>


class Node
{
    public:
        Node(Game* board, Node* parent, std::string action, float C, float prior, int visit_count);
        ~Node();

        bool is_fully_expanded();
        Node* select();
        float get_ubc(Node* child);
        void expand(xt::xtensor<float, 3> action_probs);
        void backpropagate(float value);

        // Pass this variables to private once finished debugging
        std::vector<Node*> pChildren; 
        std::string action;
        int visit_count; 
        state node_state;

    private:

        Game* game;
        float value_sum;
        Node* parent;
        float C;
        float prior;

};

struct memory_item
{
    state board_state;
    xt::xtensor<float, 3> action_probs;
    int player;
};

class SPG
{
    public:
        SPG(Game* board);
        ~SPG();

        state initial_state;
        state current_state;

        std::vector<memory_item> memory;

        Node* pRoot;
        Node* pCurrentNode;
        Game* game;

    private:


};

class MCTS
{
    public:
        MCTS(int num_searches, float dichirlet_alpha, float dichirlet_epsilon, float C);
        ~MCTS();
    
    void search(std::vector<SPG*>* spGames);

    private:
        int num_searches;
        float dichirlet_alpha;
        float dichirlet_epsilon;

        float C;

};

