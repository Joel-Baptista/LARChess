#include "mcts.h"
#include "game.h" 
#include <cmath>


MCTS::MCTS(ResNetChess* model, int num_searches, float dichirlet_alpha, float dichirlet_epsilon, float C)
{
    this->num_searches = num_searches;
    this->dichirlet_alpha = dichirlet_alpha;
    this->dichirlet_epsilon = dichirlet_epsilon;
    this->C = C;
    this->m_model = model;
}

MCTS::~MCTS()
{
}


void MCTS::search(std::vector<SPG*>* spGames)
{
    
    std::array<std::size_t, 4> state_shape = {spGames->size(), 19, 8, 8}; 
    xt::xtensor<float, 4> states(state_shape);

    for (int i = 0; i < spGames->size(); i++)
    {
        xt::view(states, i, xt::all(), xt::all(), xt::all()) = spGames->at(i)->game->get_encoded_state(spGames->at(i)->current_state);
    }

    auto output = m_model->forward(xtensor_to_torch(states));

    for (int i = 0; i < spGames->size(); i++)
    {
        std::array<std::size_t, 3> shape = {8, 8, 73}; 
        xt::xtensor<float, 3> spg_policy = torch_to_tensor(output.policy[i]);
        xt::xtensor<float, 3> valid_moves(shape);
        valid_moves = spGames->at(i)->game->get_valid_moves_encoded(spGames->at(i)->current_state);

        spg_policy *= valid_moves;
        spg_policy /= xt::sum(spg_policy)();

        spGames->at(i)->pRoot = new Node(spGames->at(i)->game, nullptr, "", C, 0.0, 1);

        spGames->at(i)->pRoot->expand(spg_policy);

    }

    for (int i = 0; i < num_searches; i++)
    {
        for (int j = 0; j < spGames->size(); j++)
        {
            spGames->at(j)->pCurrentNode = nullptr;
            Node* pNode = spGames->at(j)->pRoot;
            

            while (pNode->is_fully_expanded())
            {
                pNode = pNode->select();
            }

            final_state fState = spGames->at(j)->game->get_value_and_terminated(pNode->node_state);

            if (fState.terminated)
            {
                pNode->backpropagate(-fState.value);
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


        if (expandable_games.size() > 0)
        {
            std::array<std::size_t, 4> state_shape = {expandable_games.size(), 19, 8, 8}; 
            xt::xtensor<float, 4> states(state_shape);

            for (int i = 0; i < expandable_games.size(); i++)
            {
                int game_index = expandable_games[i];
                xt::view(states, i, xt::all(), xt::all(), xt::all()) = spGames->at(game_index)->game->get_encoded_state(spGames->at(i)->current_state);
            }

            auto output = m_model->forward(xtensor_to_torch(states));   
        }

        for (int k = 0; k < expandable_games.size(); k++)
        {
            int game_index = expandable_games[k];

            Node* pNode = spGames->at(game_index)->pCurrentNode;

            std::array<std::size_t, 3> shape = {8, 8, 73}; 
            xt::xtensor<float, 3> spg_policy = torch_to_tensor(output.policy[k]);
            
            float spg_value = output.value[k].item<float>();

            xt::xtensor<float, 3> valid_moves(shape);
            valid_moves = spGames->at(game_index)->game->get_valid_moves_encoded(pNode->node_state);

            spg_policy *= valid_moves;
            spg_policy /= xt::sum(spg_policy)();  

            pNode->expand(spg_policy);

            pNode->backpropagate(spg_value);
        }
    }     
}

Node::Node(Game* game, Node* parent, std::string action, float C, float prior, int visit_count)
{

    this->game = game;

    copy_state_from_board(node_state, game->m_Board);

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

    game->set_state(node_state);   
    auto decoded_actions = game->decode_actions(node_state, action_probs);
    for (int i = 0; i < decoded_actions.size(); i++)
    {
            std::string action = decoded_actions[i].action;
            float prob = decoded_actions[i].probability;

            copy_alpha_board(game->m_Board);

            int valid_move = game->m_Board->make_move(game->m_Board->parse_move(action.c_str()), 0);

            if (valid_move)
            {
                Node* new_node = new Node(game, this, action, C, prob, 0);
                pChildren.push_back(new_node);
            }

            restore_alpha_board(game->m_Board);

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


SPG::SPG(Game* game)
{

    this->game = game;

    copy_state_from_board(initial_state, game->m_Board);
    copy_state(current_state, initial_state);

}

SPG::~SPG()
{
}