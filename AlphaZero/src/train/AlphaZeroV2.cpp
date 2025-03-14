#include "AlphaZeroV2.h"
#include <algorithm> // For std::shuffle
#include <random>    // For std::mt19937
#include <chrono>    // For std::chrono::system_clock

AlphaZeroV2::AlphaZeroV2(
    int num_searches_init,
    int num_searches_max,
    float num_searches_ratio,
    int search_depth,
    int num_iterations,
    int warmup_iters,
    int num_selfPlay_iterations,
    int num_parallel_games,
    int num_epochs,
    int max_state_per_game,
    int swarm_update_freq,
    int batch_size,
    float early_stopping,
    float early_stopping_value,
    int buffer_size,
    float temperature,
    float temperature_decay,
    float temperature_min,
    float learning_rate_innit,
    float learning_rate_decay,
    float learning_rate_min,
    float learning_rate_update_freq,
    float dichirlet_alpha,
    float dichirlet_epsilon,
    float dichirlet_epsilon_decay,
    float dichirlet_epsilon_min,
    float C,
    float C_decay,
    float C_min,
    int num_evals,
    int eval_freq,
    int depth,
    float weight_decay,
    float dropout,
    float gradient_clip,
    int num_resblocks,
    int num_channels,
    std::string device,
    std::string pretrained_model_path,
    int num_threads,
    std::string precision_type,
    bool debug)
{

    m_logger = std::make_unique<Logger>(debug);

    this->model_path = m_logger->initLogFiles("../models/alpha");
    log_file = model_path + "/log.txt";

    if (precision_type == "float32")
    {
        precision = torch::kFloat32;
    }
    else if (precision_type == "float16")
    {
        precision = torch::kBFloat16;
    }
    else
    {
        precision = torch::kFloat32;
        m_logger->log("Precision type {" + precision_type + "} not allowed. Using float32 used by default");
    }
    
    m_Buffer = std::make_shared<ReplayBuffer>(buffer_size, precision, 100);

    if (torch::cuda::is_available() && (device.find("cuda") != device.npos))
    {
        auto dots = device.find(":");
        int device_id = 0;
        if (dots != device.npos)
        {
            device_id = std::stoi(device.substr(dots + 1, device.npos - dots));
        }

        m_Device = std::make_unique<torch::Device>(torch::kCUDA, device_id);
        m_logger->log("Using CUDA " + std::to_string(device_id));
    }
    else
    {
        m_logger->log("Using CPU");
        m_Device = std::make_unique<torch::Device>(torch::kCPU);
    }

    m_ResNetChess = std::make_shared<ResNetChess>(num_resblocks, num_channels, dropout, *m_Device);
    m_ResNetChess->to(*m_Device);
    m_ResNetChess->to(precision);

    std::cout << pretrained_model_path << std::endl;
    if (pretrained_model_path != "")
    {
        load_model(pretrained_model_path);
    }

    // m_Optimizer = std::make_unique<torch::optim::Adam>(m_ResNetChess->parameters(), torch::optim::AdamOptions(learning_rate_innit).weight_decay(weight_decay));
    m_Optimizer = std::make_unique<torch::optim::SGD>(m_ResNetChess->parameters(), torch::optim::SGDOptions(learning_rate_innit));
    // m_Scheduler = std::make_shared<torch::optim::lr_scheduler::StepLR>(*m_Optimizer, 1000, learning_rate_decay);

    for (int i = 0; i < num_threads; i++)
    {
        m_ResNetSwarm.push_back(std::make_shared<ResNetChess>(num_resblocks, num_channels, dropout, *m_Device));
        m_ResNetSwarm.at(i)->to(*m_Device, precision);
        copy_weights(*m_ResNetChess, *m_ResNetSwarm.at(i));
        network_sanity_check(*m_ResNetChess, *m_ResNetSwarm.at(i));
        m_mcts.push_back(std::make_unique<MCTS>(m_ResNetSwarm.at(i), i, num_searches_init, dichirlet_alpha, dichirlet_epsilon, C, precision));
    }

    for (int i = 0; i < num_threads; i++)
    {
        games.push_back(std::make_shared<Game>());
    }

    if (num_evals % num_threads != 0)
    {
        num_evals = num_threads * (num_evals / num_threads);
        m_logger->log("Num evals must be multiple of num threads. Setting num evals to " + std::to_string(num_evals));
    }

    this->num_searches = num_searches_init;
    this->num_searches_init = num_searches_init;
    this->num_searches_max = num_searches_max;
    this->num_searches_ratio = num_searches_ratio;
    this->search_depth = search_depth;
    this->num_iterations = num_iterations;
    this->warmup_iters = warmup_iters;
    this->num_selfPlay_iterations = num_selfPlay_iterations;
    this->buffer_size = buffer_size;
    this->num_parallel_games = num_parallel_games;
    this->num_epochs = num_epochs;
    this->max_state_per_game = max_state_per_game;
    this->swarm_update_freq = swarm_update_freq;
    this->batch_size = batch_size;
    this->early_stopping = early_stopping;
    this->early_stopping_value = early_stopping_value;
    this->temperature = temperature;
    this->temperature_decay = temperature_decay;
    this->temperature_min = temperature_min;
    this->learning_rate = learning_rate_innit;
    this->learning_rate_innit = learning_rate_innit;
    this->learning_rate_decay = learning_rate_decay;
    this->learning_rate_min = learning_rate_min;
    this->learning_rate_update_freq = learning_rate_update_freq;
    this->dichirlet_alpha = dichirlet_alpha;
    this->dichirlet_epsilon = dichirlet_epsilon;
    this->dichirlet_epsilon_decay = dichirlet_epsilon_decay;
    this->dichirlet_epsilon_min = dichirlet_epsilon_min;
    this->C = C;
    this->C_decay = C_decay;
    this->C_min = C_min;
    this->num_evals = num_evals;
    this->eval_freq = eval_freq;
    this->depth = depth;
    this->weight_decay = weight_decay;
    this->dropout = dropout;
    this->gradient_clip = gradient_clip;
    this->num_resblocks = num_resblocks;
    this->num_threads = num_threads;
    this->train_iter = 0;
    this->eval_iter = 0;
    this->debug = debug;

    lr_last_update = 0;

    logConfig();

    init_envs();
}

AlphaZeroV2::~AlphaZeroV2()
{
}

void AlphaZeroV2::update_dichirlet()
{
    dichirlet_epsilon = std::max(dichirlet_epsilon * dichirlet_epsilon_decay, dichirlet_epsilon_min);

    for (int i = 0; i < num_threads; i++)
        m_mcts.at(i)->set_dichirlet_epsilon(dichirlet_epsilon);

    m_logger->log("Dichirlet epsilon: " + std::to_string(dichirlet_epsilon) + " Dichirlet alpha: " + std::to_string(dichirlet_alpha));
}

void AlphaZeroV2::update_temperature()
{
    temperature = std::max(temperature * temperature_decay, temperature_min);

    m_logger->log("Temperature: " + std::to_string(temperature));
}

void AlphaZeroV2::update_C()
{
    C = std::max(C * C_decay, C_min);

    for (int i = 0; i < num_threads; i++)
        m_mcts.at(i)->set_C(C);

    m_logger->log("C: " + std::to_string(C));
}

void AlphaZeroV2::update_num_searches()
{
    num_searches = std::min((int)(num_searches * num_searches_ratio), num_searches_max);

    for (int i = 0; i < num_threads; i++)
        m_mcts.at(i)->set_num_searches(num_searches);

    m_logger->log("num_searches: " + std::to_string(num_searches));
}

void AlphaZeroV2::update_learning_rate()
{
    if (lr_last_update + learning_rate_update_freq < train_iter)
    {
        lr_last_update += learning_rate_update_freq;

        learning_rate = std::max((learning_rate * learning_rate_decay), learning_rate_min);
        for (auto &param_group : m_Optimizer->param_groups())
        {
            static_cast<torch::optim::AdamOptions &>(param_group.options()).lr(learning_rate);
        }
    }
}

void AlphaZeroV2::init_envs()
{
    for (int i = 0; i < num_threads; i++)
    {
        m_envs.push_back(std::make_unique<Environment>(
            i,
            m_ResNetSwarm.at(i),
            m_mcts.at(i),
            num_parallel_games,
            m_Buffer,
            early_stopping,
            early_stopping_value,
            precision
            ));
    }
}

void AlphaZeroV2::env_step(int env_id)
{
    m_envs.at(env_id)->step();
}

void AlphaZeroV2::learn()
{
    int st = get_time_ms();
    int st_total = get_time_ms();

    int num_games_playing = num_threads * num_parallel_games;

    for (int i = 0; i < num_iterations / num_games_playing; i++)
    {
        int st = get_time_ms();
        std::vector<std::future<void>> futures;

        for (int j = 0; j < num_threads; ++j)
        {
            futures.push_back(std::async(std::launch::async, &AlphaZeroV2::env_step, this, j));
        }

        if (m_Buffer->size() < batch_size || m_Buffer->size() < warmup_iters)
        {
            for (auto &future : futures) // env step and network train work in parallel
            {
                try
                {
                    future.get(); // Retrieve result once
                }
                catch (const std::exception &e)
                {
                    m_logger->log("Exception caught during future.get(): " + std::string(e.what()));
                }
            }
            
            float time_taken = ((float)(get_time_ms() - st) / (1000.0f));
            m_logger->logMessage(std::to_string(train_iter) + "," + std::to_string(time_taken / num_games_playing), model_path + "/fps.csv");
            continue;
        }

        bool evalTime = false;
        
        {// Automatically delete global variables
            // Allocation one time the memory might speed up GPU utilization
            auto global_states = torch::empty({batch_size, 19, 8, 8},
                torch::TensorOptions().dtype(precision).pinned_memory(true)).to(*m_Device); // Allocate once
            auto global_actions = torch::empty({batch_size, 8, 8, 73},
                torch::TensorOptions().dtype(precision).pinned_memory(true)).to(*m_Device); // Allocate once
            auto global_values = torch::empty({batch_size},
                torch::TensorOptions().dtype(precision).pinned_memory(true)).to(*m_Device); // Allocate once

            for (int k = 0; k < num_games_playing; k++) // Take as many gradient steps as environment steps (1:1 ratio)
            {
                train(global_states, global_actions, global_values);
                if (train_iter % eval_freq == 0)
                {
                    evalTime = true;
                }
            }
        }

        for (auto &future : futures) // env step and network train work in parallel
        {
            try
            {
                future.get(); // Retrieve result once
            }
            catch (const std::exception &e)
            {
                m_logger->log("Exception caught during future.get(): " + std::string(e.what()));
            }
        }

        save_model(model_path);

        float time_taken = ((float)(get_time_ms() - st) / (1000.0f));
        m_logger->logMessage(std::to_string(train_iter) + "," + std::to_string(time_taken / num_games_playing), model_path + "/fps.csv");

        if (evalTime)
        {
            std::vector<std::future<int>> futuresEval;
            evalResults results;
            st = get_time_ms();
            for (int i = 0; i < (num_evals / num_threads); i++)
            {
                // Eval the bot
                for (int j = 0; j < num_threads; ++j)
                {
                    futuresEval.push_back(std::async(std::launch::async, &AlphaZeroV2::AlphaEval, this, j, depth));
                }
                for (auto &future : futuresEval)
                {
                    try
                    {
                        auto result = future.get(); // Retrieve result once
                        if (result == 1)
                            results.win_count++;
                        else if (result == 0)
                            results.draw_count++;
                        else
                            results.loss_count++;
                    }
                    catch (const std::exception &e)
                    {
                        m_logger->log("Exception caught during future.get(): " + std::string(e.what()));
                    }
                }
                futuresEval.clear(); // Clear the futures vector before the next iteration
            }

            m_logger->log("Wins %: " + std::to_string(results.win_count / (float)num_evals) + "%, " +
                          "Loss %: " + std::to_string(results.loss_count / (float)num_evals) + "%, " +
                          "Draw %: " + std::to_string(results.draw_count / (float)num_evals) +
                          "%, Eval Time: " + std::to_string((float)(get_time_ms() - st) / 1000.0f) + " seconds");

            m_logger->logEval(std::to_string(eval_iter) + "," +
                              std::to_string(results.win_count / (float)num_evals) + "," +
                              std::to_string(results.loss_count / (float)num_evals) + "," +
                              std::to_string(results.draw_count / (float)num_evals));
            eval_iter++;

            if (results.win_count / num_evals > 0.5)
            {
                depth = (depth + 1 > 5) ? 5 : depth + 1;
                m_logger->log("Depth increased to: " + std::to_string(depth));
            }
        }

        m_logger->logMessage(std::to_string(train_iter) + "," + std::to_string(m_Buffer->get_mean_len()), model_path + "/ep_len.csv");
        m_logger->logMessage(std::to_string(train_iter) + "," + std::to_string(m_Buffer->get_mean_res()), model_path + "/ep_res.csv");
        m_logger->logMessage(std::to_string(train_iter) + "," + std::to_string(m_Buffer->get_n_games()), model_path + "/n_games.csv");
        m_logger->logMessage(std::to_string(train_iter) + "," + std::to_string(m_Buffer->get_num_steps()), model_path + "/env_steps.csv");
    }
}

void AlphaZeroV2::train(torch::Tensor global_state, torch::Tensor global_action, torch::Tensor global_value)
{
    auto st = get_time_ms();
    auto st_aux = get_time_ms();

    torch::cuda::synchronize();

    m_logger->log("Synchronize mode: " + std::to_string(get_time_ms() - st_aux) + " ms");
    st_aux = get_time_ms();
    m_ResNetChess->train();
    torch::cuda::synchronize();
    m_logger->log("Change to train mode: " + std::to_string(get_time_ms() - st_aux) + " ms");
    st_aux = get_time_ms();

    float running_loss = 0.0;
    float batch_count = 0.0;

    auto samples = m_Buffer->sample(batch_size, max_state_per_game);

    m_logger->log("Sample from batch: " + std::to_string(get_time_ms() - st_aux) + " ms");
    st_aux = get_time_ms();

    global_state.copy_(samples.states);
    global_action.copy_(samples.action_probs);
    global_value.copy_(samples.values);
    torch::cuda::synchronize();

    m_logger->log("To device: " + std::to_string(get_time_ms() - st_aux) + " ms");
    st_aux = get_time_ms();

    auto output = m_ResNetChess->forward(global_state);
    torch::cuda::synchronize();
    m_logger->log("Forward Pass: " + std::to_string(get_time_ms() - st_aux) + " ms");
    st_aux = get_time_ms();

    if (torch::any(torch::isnan(global_action)).item<bool>() || torch::any(torch::isnan(output.policy)).item<bool>())
    {
        std::cerr << "NaN detected in encoded_actions or output!" << std::endl;
        throw std::runtime_error("NaN detected in encoded_actions or output.");
    }
    torch::cuda::synchronize();
    m_logger->log("Isnan verifictiaon: " + std::to_string(get_time_ms() - st_aux) + " ms");
    st_aux = get_time_ms();

    torch::Tensor policy_loss = torch::nn::functional::cross_entropy(output.policy, global_action);

    auto value_loss = torch::nn::functional::mse_loss(output.value.squeeze(1), global_value);

    auto loss = policy_loss + value_loss;
    torch::cuda::synchronize();
    m_logger->log("Loss Calculation: " + std::to_string(get_time_ms() - st_aux) + " ms");
    st_aux = get_time_ms();

    m_Optimizer->zero_grad();

    loss.backward();
    torch::cuda::synchronize();
    m_logger->log("Backpropagation: " + std::to_string(get_time_ms() - st_aux) + " ms");
    st_aux = get_time_ms();

    double grad_norm = calculate_gradient_norm(m_ResNetChess->parameters());
    m_logger->logGrad(std::to_string(train_iter) + "," + std::to_string(grad_norm));

    torch::nn::utils::clip_grad_norm_(m_ResNetChess->parameters(), gradient_clip);
    torch::cuda::synchronize();
    m_logger->log("Grad Clipping: " + std::to_string(get_time_ms() - st_aux) + " ms");
    st_aux = get_time_ms();

    m_Optimizer->step();
    torch::cuda::synchronize();
    m_logger->log("Optimizer time: " + std::to_string(get_time_ms() - st_aux) + " ms");
    st_aux = get_time_ms();

    // running_loss += loss.mean().cpu().item<float>();
    batch_count++;
    samples.states.reset();
    samples.action_probs.reset();
    samples.values.reset();

    m_logger->logTrain(std::to_string(train_iter) + "," + std::to_string(loss.cpu().item<float>()));
    m_logger->logMessage(std::to_string(train_iter) + "," + std::to_string(policy_loss.cpu().item<float>()), model_path + "/pi_loss.csv");
    m_logger->logMessage(std::to_string(train_iter) + "," + std::to_string(value_loss.cpu().item<float>()), model_path + "/val_loss.csv");
    m_logger->logMessage(std::to_string(train_iter) + "," + std::to_string(m_Optimizer->param_groups().at(0).options().get_lr()), model_path + "/lr.csv");
    train_iter++;
    torch::cuda::synchronize();
    m_logger->log("Log info: " + std::to_string(get_time_ms() - st_aux) + " ms");
    st_aux = get_time_ms();

    if (train_iter % swarm_update_freq == 0)
    {
        for (int i = 0; i < num_threads; i++)
        {

            m_ResNetSwarm.at(i)->eval();
            copy_weights(*m_ResNetChess, *m_ResNetSwarm.at(i));
            network_sanity_check(*m_ResNetChess, *m_ResNetSwarm.at(i));
        }

        m_logger->log("Data Aquisition Nets Updated");
    }

    update_learning_rate();
    torch::cuda::synchronize();
    m_logger->log("Auxiliar Updates: " + std::to_string(get_time_ms() - st_aux) + " ms");

    m_logger->log(" Loss: " + std::to_string(running_loss / (float)batch_count) + " Time: " + std::to_string((float)(get_time_ms() - st) / 1000.0f) + " seconds");
    m_logger->log("-------------------------------------------------------------");
}

void AlphaZeroV2::network_sanity_check(ResNetChess &source, ResNetChess &target)
{

    for (auto &p : target.parameters())
    {
        if (torch::any(torch::isnan(p)).item<bool>() || torch::any(torch::isinf(p)).item<bool>())
        {
            m_logger->log("NaN or Inf found in the network");
            throw std::runtime_error("NaN or Inf found in the network");
        }
    }

    std::vector<int64_t> shape = {1, 19, 8, 8};
    torch::Tensor encoded_state = torch::rand(shape, precision).to(*m_Device); // Initialize the tensor with zeros
    source.eval();
    target.eval();

    auto output1 = source.forward(encoded_state);
    auto output2 = target.forward(encoded_state);

    if (torch::any(torch::isnan(output1.policy)).item<bool>() || torch::any(torch::isinf(output1.policy)).item<bool>())
    {
        m_logger->log("Source policy has NaN or Inf values");
        // throw std::runtime_error("Source policy has NaN or Inf values");
    }

    if (torch::any(torch::isnan(output1.value)).item<bool>() || torch::any(torch::isinf(output1.value)).item<bool>())
    {
        m_logger->log("Source value has NaN or Inf values");
        // throw std::runtime_error("Source value has NaN or Inf values");
    }

    if (torch::any(torch::isnan(output2.policy)).item<bool>() || torch::any(torch::isinf(output2.policy)).item<bool>())
    {
        m_logger->log("Target policy has NaN or Inf values");
        // throw std::runtime_error("Target policy has NaN or Inf values");
    }

    if (torch::any(torch::isnan(output2.value)).item<bool>() || torch::any(torch::isinf(output2.value)).item<bool>())
    {
        m_logger->log("Target value has NaN or Inf values");
        // throw std::runtime_error("Target value has NaN or Inf values");
    }

    if (torch::allclose(output1.policy, output2.policy, 1e-5, 1e-6) &&
        torch::allclose(output1.value, output2.value, 1e-5, 1e-6))
    {
        // m_logger->log("Models are identical");
    }
    else
    {
        m_logger->log("The two networks are not outputting the same results");

        // Log differences
        m_logger->log("Max difference in policy: " + std::to_string((output1.policy - output2.policy).abs().max().item<float>()));
        m_logger->log("Max difference in value: " + std::to_string((output1.value - output2.value).abs().max().item<float>()));
    }
}

int AlphaZeroV2::AlphaEval(int thread_id, int depth)
{
    std::vector<SPG *> spGames;
    evalResults results;

    m_ResNetSwarm.at(thread_id)->eval();

    games.at(thread_id)->m_Board->parse_fen(start_position);
    SPG *spg = new SPG(games.at(thread_id), 0.0);
    spGames.push_back(spg);

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<> dis(0.0, 1.0);
    double random_number = dis(gen);

    int alpha_white;
    if (random_number < 0.5)
    {
        alpha_white = 1;
    }
    else
    {
        alpha_white = 0;
    }

    while (true)
    {
        if (spGames.at(0)->game->m_Board->get_side() == alpha_white)
        {
            copy_alpha_board(spGames.at(0)->game->m_Board);

            m_mcts.at(thread_id)->search(&spGames);

            restore_alpha_board(spGames.at(0)->game->m_Board);

            std::vector<int64_t> shape = {8, 8, 73};
            torch::Tensor action_probs = torch::zeros(shape, precision); // Initialize the tensor with zeros

            std::vector<int> visited_childs;
            for (int j = 0; j < spGames.at(0)->pRoot->pChildren.size(); j++)
            {
                if (spGames.at(0)->pRoot->pChildren.at(j)->visit_count > 0)
                    visited_childs.push_back(j);

                action_probs +=
                    spGames.at(0)->game->get_encoded_action(spGames.at(0)->pRoot->pChildren.at(j)->action, spGames.at(0)->current_state.side) * spGames.at(0)->pRoot->pChildren.at(j)->visit_count;
            }

            action_probs /= action_probs.sum();

            std::string move = spGames.at(0)->game->decode_action(spGames.at(0)->current_state, action_probs);

            games.at(thread_id)->m_Board->make_player_move(move.c_str());
            delete spGames.at(0)->pRoot;
        }
        else
        {
            games.at(thread_id)->m_Board->reset_leaf_nodes();
            float eval = games.at(thread_id)->m_Board->alpha_beta(depth, -1000000, 1000000, true);
            games.at(thread_id)->m_Board->make_bot_move(games.at(thread_id)->m_Board->get_bot_best_move());
        }

        state current_state = games.at(thread_id)->get_state();
        spGames.at(0)->current_state = current_state;

        final_state fState = spGames.at(0)->game->get_value_and_terminated(current_state, spGames.at(0)->repeated_states);

        if (fState.terminated)
        {
            int res;
            if ((current_state.side == alpha_white) && (fState.value == 1.0))
            {
                res = -1;
            }
            else if ((current_state.side != alpha_white) && (fState.value == 1.0))
            {
                res = 1;
            }
            else
            {
                res = 0;
            }
            delete spGames.at(0);
            spGames.erase(spGames.begin());
            return res;
        }
    }

    return 0;
}

void AlphaZeroV2::save_model(std::string path)
{
    if (!debug)
    {
        if (path == "")
        {
            torch::save(m_ResNetChess, "model.pt");
            // m_logger->log( "Model saved in: model.pt" );
        }
        else
        {
            torch::save(m_ResNetChess, path + "/model.pt");
            // m_logger->log( "Model saved in: " + path + "/model.pt" );
        }
    }
}

void AlphaZeroV2::load_model(std::string path)
{
    std::vector<int64_t> shape = {1, 19, 8, 8};
    torch::Tensor encoded_state = torch::rand(shape, precision).to(*m_Device); // Initialize the tensor with zeros
    torch::load(m_ResNetChess, path + "model.pt", *m_Device);
    m_ResNetChess->to(precision);
    m_ResNetChess->eval();

    auto st = get_time_ms();
    auto output1 = m_ResNetChess->forward(encoded_state);
    std::cout << "Before clamp: " << ((get_time_ms() - st) / 1000.0f) << " seconds" << std::endl;

    m_ResNetChess->to(*m_Device, precision);

    // clamp_small_weights(*m_ResNetChess, 1e-15);

    st = get_time_ms();
    auto output2 = m_ResNetChess->forward(encoded_state);
    std::cout << "After clamp: " << ((get_time_ms() - st) / 1000.0f) << " seconds" << std::endl;

    if (torch::allclose(output1.policy, output2.policy) || torch::allclose(output1.value, output2.value))
    {
        std::cout << "Model loaded successfully" << std::endl;
    }
    else
    {
        std::cout << "Be carefull! Clamping the weights significantly altered the network" << std::endl;
    }
}

void AlphaZeroV2::update_hyper()
{
    update_dichirlet();
    update_temperature();
    update_C();
    update_num_searches();
}

void AlphaZeroV2::log(std::string message)
{
    std::cout << "[" << getCurrentTimestamp() << "] " << message << std::endl;

    if (debug)
        return;

    m_logger->logMessage("[" + getCurrentTimestamp() + "] " + message, model_path + "/log.txt");
}

void AlphaZeroV2::logTrain(std::string message)
{
    if (debug)
        return;
    m_logger->logMessage(message, model_path + "/train.csv");
}

void AlphaZeroV2::logEval(std::string message)
{
    if (debug)
        return;
    m_logger->logMessage(message, model_path + "/eval.csv");
}

void AlphaZeroV2::logConfig()
{
    std::unordered_map<std::string, std::string> config;

    config["num_searches_init"] = std::to_string(num_searches_init);
    config["num_searches_max"] = std::to_string(num_searches_max);
    config["num_searches_ratio"] = std::to_string(num_searches_ratio);
    config["search_depth"] = std::to_string(search_depth);
    config["num_iterations"] = std::to_string(num_iterations);
    config["warmup_iters"] = std::to_string(warmup_iters);
    config["num_selfPlay_iterations"] = std::to_string(num_selfPlay_iterations);
    config["num_parallel_games"] = std::to_string(num_parallel_games);
    config["num_epochs"] = std::to_string(num_epochs);
    config["max_state_per_game"] = std::to_string(max_state_per_game);
    config["swarm_update_freq"] = std::to_string(swarm_update_freq);
    config["batch_size"] = std::to_string(batch_size);
    config["early_stopping"] = std::to_string(early_stopping);
    config["early_stopping_value"] = std::to_string(early_stopping_value);
    config["buffer_size"] = std::to_string(buffer_size);
    config["temperature"] = std::to_string(temperature);
    config["temperature_decay"] = std::to_string(temperature_decay);
    config["temperature_min"] = std::to_string(temperature_min);
    config["learning_rate_innit"] = std::to_string(learning_rate_innit);
    config["learning_rate_decay"] = std::to_string(learning_rate_decay);
    config["learning_rate_min"] = std::to_string(learning_rate_min);
    config["learning_rate_update_freq"] = std::to_string(learning_rate_update_freq);
    config["dichirlet_alpha"] = std::to_string(dichirlet_alpha);
    config["dichirlet_epsilon"] = std::to_string(dichirlet_epsilon);
    config["dichirlet_epsilon_decay"] = std::to_string(dichirlet_epsilon_decay);
    config["dichirlet_epsilon_min"] = std::to_string(dichirlet_epsilon_min);
    config["C"] = std::to_string(C);
    config["C_decay"] = std::to_string(C_decay);
    config["C_min"] = std::to_string(C_min);
    config["num_evals"] = std::to_string(num_evals);
    config["eval_freq"] = std::to_string(eval_freq);
    config["depth"] = std::to_string(depth);
    config["weight_decay"] = std::to_string(weight_decay);
    config["dropout"] = std::to_string(dropout);
    config["gradient_clip"] = std::to_string(gradient_clip);
    config["num_resblocks"] = std::to_string(num_resblocks);
    config["num_threads"] = std::to_string(num_threads);

    m_logger->logConfig(config);
}