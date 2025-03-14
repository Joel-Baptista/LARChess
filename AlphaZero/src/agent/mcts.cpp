#include "../headers/mcts.h"
#include "../headers/game.h" 
#include <cmath>
#include <omp.h>



MCTS::MCTS(std::shared_ptr<ResNetChess> model, int num_searches, float dichirlet_alpha, float dichirlet_epsilon, float C, c10::ScalarType precision)
{
    this->num_searches = num_searches;
    this->dichirlet_alpha = dichirlet_alpha;
    this->dichirlet_epsilon = dichirlet_epsilon;
    this->C = C;
    this->m_model = model;
    this->thread_id = 0;
    this->precision = precision;
    this->boards_visited.clear();
}
MCTS::MCTS(std::shared_ptr<ResNetChess> model, int thread_id, int num_searches, float dichirlet_alpha, float dichirlet_epsilon, float C, c10::ScalarType precision)
{
    this->num_searches = num_searches;
    this->dichirlet_alpha = dichirlet_alpha;
    this->dichirlet_epsilon = dichirlet_epsilon;
    this->C = C;
    this->m_model = model;
    this->thread_id = thread_id;
    this->precision = precision;
    this->boards_visited.clear();
}

MCTS::~MCTS()
{
}

std::vector<std::tuple<torch::Tensor, float>> MCTS::predict(std::vector<SPG*>* spGames, bool deterministic)
{

    this->search(spGames);

    std::vector<std::tuple<torch::Tensor, float>> results;

    for (int i = 0; i < spGames->size(); i++)
    {

        std::vector<int64_t> shape = {8, 8, 73};
        torch::Tensor action_probs = torch::zeros(shape, precision); // Initialize the tensor with zeros
        
        std::vector<int> visited_childs;
        std::vector<float> visits_counts;
        std::vector<float> visit_probs;

        float state_value = 0.0f;
        // std::cout << "Visit Counts: [";
        for (int j = 0; j < spGames->at(i)->pRoot->pChildren.size(); j++)
        {
            // std::cout << spGames.at(i)->pRoot->pChildren.at(j)->visit_count << " ";
            if (spGames->at(i)->pRoot->pChildren.at(j)->visit_count > 0)
            {
                visited_childs.push_back(j);
            }

            visit_probs.push_back((float)spGames->at(i)->pRoot->pChildren.at(j)->visit_count / this->get_num_searches());

            state_value += (spGames->at(i)->pRoot->pChildren.at(j)->value_sum / (float)spGames->at(i)->pRoot->pChildren.at(j)->visit_count) * visit_probs[j];
        }

        for (int j = 0; j < spGames->at(i)->pRoot->pChildren.size(); j++)
        {
            action_probs += 
                spGames->at(i)->game->get_encoded_action(spGames->at(i)->pRoot->pChildren.at(j)->action, spGames->at(i)->current_state.side) 
                * visit_probs[j];
        }

        results.push_back(std::make_tuple(action_probs, state_value));
    }

    return results;
}


void MCTS::search(std::vector<SPG*>* spGames, bool deterministic)
{
    m_model->eval();
    boards_visited.clear();
    auto st = get_time_ms();
    std::vector<int64_t> shape = {(long)spGames->size(), 19, 8, 8};
    torch::Tensor encoded_state = torch::zeros(shape, precision); // Initialize the tensor with zeros
    for (int i = 0; i < spGames->size(); i++)
    {
        std::vector<int64_t> shape = {(long)spGames->size(), 19, 8, 8};
        torch::Tensor state = torch::zeros({1, 19, 8, 8}, precision); // Initialize the tensor with zeros
        get_encoded_state(state, spGames->at(i)->current_state);
        encoded_state[i] = state.squeeze(0); 
    }

    chess_output output_roots;
    {
        torch::NoGradGuard no_grad;

        encoded_state = encoded_state.to(*m_model->m_Device);
        output_roots = m_model->forward(encoded_state);
        int batch_size = output_roots.policy.size(0);
        
        output_roots.policy = torch::softmax(output_roots.policy.view({output_roots.policy.size(0), -1}), 1).view({-1, 8, 8, 73});
        output_roots.policy = output_roots.policy.cpu();
        output_roots.value = output_roots.value.cpu();

    }

    for (int i = 0; i < spGames->size(); i++)
    {
        std::vector<int64_t> shape = {8, 8, 73}; 
        torch::Tensor spg_policy = output_roots.policy[i];
        torch::Tensor valid_moves = torch::zeros(shape, precision);

        spGames->at(i)->game->set_state(spGames->at(i)->current_state);

        moves move_list;
        spGames->at(i)->game->m_Board->get_alpha_moves(&move_list);

        get_valid_moves_encoded(valid_moves, spGames->at(i)->current_state, move_list);
        spg_policy *= valid_moves;
        spg_policy /= spg_policy.sum();

        if (!deterministic)
        {
            std::vector<double> alpha(move_list.count, dichirlet_alpha);
            add_dirichlet_noise(spg_policy, alpha, dichirlet_epsilon);
            spg_policy /= spg_policy.sum();
        }

        spGames->at(i)->pRoot = new Node(spGames->at(i)->game, nullptr, "", C, 0.0, 1);

        spGames->at(i)->pRoot->expand(spg_policy, valid_moves);
    }

    int inputs_time = 0;

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

            // boards_visited[pNode->node_state.bitboards] += 1;

            final_state fState = spGames->at(j)->game->get_value_and_terminated(pNode->node_state, spGames->at(j)->repeated_states);
            
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

        chess_output output_exapandables;
        if (expandable_games.size() > 0)
        {
            std::vector<int64_t> exp_shape = {(long)expandable_games.size(), 19, 8, 8};
            torch::Tensor encoded_states = torch::zeros(exp_shape, precision); // Initialize the tensor with zeros

            #pragma omp parallel for
            for (int i = 0; i < expandable_games.size(); i++)
            {
                int game_index = expandable_games[i];
                torch::Tensor state = torch::zeros({1, 19, 8, 8}, precision); // Initialize the tensor with zeros
                get_encoded_state(state, spGames->at(game_index)->pCurrentNode->node_state);
                encoded_states[i] = state.squeeze(0); 
            }
            {
                torch::NoGradGuard no_grad;

                encoded_states = encoded_states.to(*m_model->m_Device);
                output_exapandables = m_model->forward(encoded_states);

                output_exapandables.policy = torch::softmax(output_exapandables.policy.view({output_exapandables.policy.size(0), -1}), 1).view({-1, 8, 8, 73});
                output_exapandables.policy = output_exapandables.policy.cpu();
                output_exapandables.value = output_exapandables.value.cpu();
            }

        }
        
        #pragma omp parallel for
        for (int k = 0; k < expandable_games.size(); k++)
        {
            
            int game_index;
            torch::Tensor spg_policy;
            
            {
                game_index = expandable_games[k];
                spg_policy = output_exapandables.policy[k];
            }
            std::vector<int64_t> shape = {8, 8, 73}; 
            torch::Tensor valid_moves = torch::zeros(shape, precision);
            moves move_list;

            spGames->at(game_index)->game->set_state(spGames->at(game_index)->pCurrentNode->node_state);
            spGames->at(game_index)->game->m_Board->get_alpha_moves(&move_list);
        
            get_valid_moves_encoded(valid_moves, spGames->at(game_index)->pCurrentNode->node_state, move_list);
            
            spg_policy *= valid_moves;
            spg_policy /= spg_policy.sum();

            if (!spGames->at(game_index)->pCurrentNode->is_fully_expanded())
            {
                spGames->at(game_index)->pCurrentNode->expand(spg_policy, valid_moves);
            }

            spGames->at(game_index)->pCurrentNode->backpropagate(output_exapandables.value[k].item<float>());

        } 
    }   
}

Node::Node(std::shared_ptr<Game> game, Node* parent, std::string action, float C, float prior, int visit_count)
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
    for (int i = 0; i < pChildren.size(); i++)
    {
        delete pChildren[i];
    }
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

    // copy_alpha_board(game->m_Board);
    // game->set_state(node_state);
    // game->m_Board->print_board();
    // restore_alpha_board(game->m_Board);

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
        q_value = (child->value_sum / child->visit_count);
    }


    return q_value + child->prior * C * std::sqrt(visit_count) / (1 + child->visit_count);

}
void Node::expand(torch::Tensor action_probs, torch::Tensor valid_moves)
{

    game->set_state(node_state);  
    auto st = get_time_ms();
    auto decoded_actions = game->decode_actions(node_state, action_probs, valid_moves);

    int count = 0;

    for (int i = 0; i < decoded_actions.size(); i++)
    {
            std::string action = decoded_actions[i].action;
            float prob = decoded_actions[i].probability;
            
            copy_alpha_board(game->m_Board);

            int valid_move = game->m_Board->make_move(game->m_Board->parse_move(action.c_str()));

            if (valid_move)
            {   
                count++;
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


SPG::SPG(std::shared_ptr<Game> game, float early_stopping)
{

    this->game = game;

    copy_state_from_board(initial_state, game->m_Board);
    copy_state(current_state, initial_state);

    if (get_prob_uni() < early_stopping)
    {
        early_stop = false;
    }
    else
    {
        early_stop = true;
    }

}

SPG::~SPG()
{
}

void SPG::reset()
{
    copy_state(current_state, initial_state);
    pRoot = nullptr;
    pCurrentNode = nullptr;
    repeated_states.clear();
    memory.clear();

    if (get_prob_uni() < 0.2)
    {
        early_stop = false;
    }
    else
    {
        early_stop = true;
    }

}
