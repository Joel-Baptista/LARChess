#include "mcts.h"
#include "game.h" 
#include <cmath>


MCTS::MCTS(int num_searches, float dichirlet_alpha, float dichirlet_epsilon, float C)
{
    this->num_searches = num_searches;
    this->dichirlet_alpha = dichirlet_alpha;
    this->dichirlet_epsilon = dichirlet_epsilon;
    this->C = C;
}

MCTS::~MCTS()
{
}


void MCTS::search(std::vector<SPG*>* spGames)
{
    
    
    std::vector<state> node_states;

    for (int i = 0; i < spGames->size(); i++)
    {
        node_states.push_back(spGames->at(i)->current_state);
    }

    /*
    
        TORCH INFERENCE CODE GOES HERE

    */

    std::array<std::array<float, 1 * 73 * 8 * 8>, 150> action_probs = {2};

    for (int i = 0; i < spGames->size(); i++)
    {
        std::array<std::size_t, 3> shape = {8, 8, 73}; 
        xt::xtensor<float, 3> spg_policy = xt::adapt(action_probs[i].data(), shape); 

        spg_policy.fill(1.0f);
        xt::xtensor<float, 3> valid_moves(shape);
        valid_moves.fill(2.0f);

        spg_policy *= valid_moves;
        spg_policy /= xt::sum(spg_policy)();

        spGames->at(i)->pRoot = new Node(spGames->at(i)->game, nullptr, "e2e4", C, 0.0, 1);

        spGames->at(i)->pRoot->expand(spg_policy);
    }

    std::cout << "Roots Expanded" << std::endl;

    for (int i = 0; i < num_searches; i++)
    {
        std::cout << "Search: " << i + 1 << std::endl;
        for (int j = 0; j < spGames->size(); j++)
        {
            // std::cout << "Game: " << j + 1 << std::endl;
            spGames->at(j)->pCurrentNode = nullptr;
            Node* pNode = spGames->at(j)->pRoot;
            

            while (pNode->is_fully_expanded())
            {
                pNode = pNode->select();
            }

            /*
            
                Get value of the state and if it is a terminal state

            */ 

            bool terminal = false;
            float value = 0;

            if (terminal)
            {
                pNode->backpropagate(value);
            }
            else
            {
                spGames->at(j)->pCurrentNode = pNode;
            }    

        }

        std::vector<int> expandable_games;

        for (int k = 0; k < spGames->size(); k++)
        {
            if (spGames->at(k)->pCurrentNode != nullptr)
            {
                expandable_games.push_back(k);
            }
        }

        std::array<std::array<float, 1 * 73 * 8 * 8>, 150> policy = {2};
        std::array<float, 150> values = {2};

        if (expandable_games.size() > 0)
        {
            /*
            
                TORCH INFERENCE CODE GOES HERE
            
            */   
        }

        for (int k = 0; k < expandable_games.size(); k++)
        {
            // std::cout << "Expanding Game: " << k + 1 << std::endl;
            int game_index = expandable_games[k];

            Node* pNode = spGames->at(game_index)->pCurrentNode;

            std::array<std::size_t, 3> shape = {8, 8, 73}; 
            xt::xtensor<float, 3> spg_policy = xt::adapt(policy[k].data(), shape);
            
            float spg_value = values[k];

            xt::xtensor<float, 3> valid_moves(shape);
            valid_moves.fill(2.0f);

            spg_policy *= valid_moves;
            spg_policy /= xt::sum(spg_policy)();  

            pNode->expand(spg_policy);

            pNode->backpropagate(spg_value);
            // std::cout << "Reached this place" << std::endl;
        }
    }

}

Node::Node(BitBoard* board, Node* parent, std::string action, float C, float prior, int visit_count)
{

    game = board->clone(); // Game is a pointer to the clone

    copy_state_from_board(node_state, board);

    this->parent = parent;
    this->action = action;
    this->C = C;
    this->prior = prior;
    this->visit_count = visit_count;
    this->value_sum = 0;
}

Node::~Node()
{
    delete game;
}

bool Node::is_fully_expanded()
{
    return pChildren.size() > 0;
}
Node* Node::select()
{
    Node* selected_child = nullptr;
    float best_ubc = -1000000;


    for (int i = 0; i < pChildren.size(); i++)
    {
        Node* child = pChildren[i];

        float ubc = get_ubc(child);

        if (ubc > best_ubc)
        {
            best_ubc = ubc;
            selected_child = child;
        }
    }

    return selected_child;
}
float Node::get_ubc(Node* child)
{
    float q_value;

    if (child->visit_count == 0)
    {
        q_value = 0;
    }
    else
    {
        q_value = ((child->value_sum / child->visit_count) + 1.0f) / 2.0f;
    }

    return q_value + child->prior * C * std::sqrt(visit_count) / (1 + child->visit_count);

}
void Node::expand(xt::xtensor<float, 3> action_probs)
{

    /*
        Code for turning action_probs into a list of actions and probabilities
    */

   std::vector<std::string> actions;
   std::vector<float> probs;

   for (int i = 0; i < actions.size(); i++)
   {
        std::string action = actions[i];

        // state new_state;
        // copy_state(new_state, node_state);

        copy_alpha_board(game);

        int valid_move = game->make_move(game->parse_move(action.c_str()), 0);

        if (valid_move)
        {
            Node* new_node = new Node(game, this, action, C, probs[i], 0);
            pChildren.push_back(new_node);
        }

        restore_alpha_board(game);

   }

}
void Node::backpropagate(float value)
{
    visit_count++;
    value_sum += value;

    if (parent != nullptr)
    {   
        parent->backpropagate(-value);
    }
}


SPG::SPG(BitBoard* board)
{

    game = board->clone(); // Game is a pointer to the clone

    copy_state_from_board(initial_state, board);
    copy_state(current_state, initial_state);

}

SPG::~SPG()
{
}