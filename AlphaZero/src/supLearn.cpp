#include "supLearn.h"


#include "include/json.hpp"

using json = nlohmann::json;

SupervisedLearning::SupervisedLearning(
    std::string& dataset_path,
    int num_epochs, 
    float learning_rate,
    int batch_size,
    float train_split,
    float weight_decay,
    float dropout,
    int num_resblocks,
    int num_channels,
    std::string device,
    std::string model_name,
    bool hasHeaders = true
    ) 
{
    this->model_path = initLogFiles("../models/slearn");
    log_file = model_path + "/log.txt";

    m_dataset = std::make_shared<Dataset>(dataset_path, train_split, hasHeaders);
    m_dataset->shuffle();
    logMessage("Train size: " + std::to_string(m_dataset->train_size()), log_file);
    logMessage("Eval size: " + std::to_string(m_dataset->eval_size()), log_file);
    
    if (torch::cuda::is_available() && (device.find("cuda") != device.npos))
    {

        auto dots = device.find(":");
        int device_id = 0;
        if (dots != device.npos)
        {
            device_id = std::stoi(device.substr(dots + 1, device.npos - dots));
        }

        m_Device = std::make_unique<torch::Device>(torch::kCUDA, device_id);
        logMessage("Using CUDA " + std::to_string(device_id), log_file);
    }
    else
    {
        logMessage("Using CPU", log_file);
        m_Device = std::make_unique<torch::Device>(torch::kCPU);
    }

    m_ResNetChess = std::make_shared<ResNetChess>(num_resblocks, num_channels, dropout,*m_Device);
    
    m_Optimizer = std::make_unique<torch::optim::Adam>(m_ResNetChess->parameters(), torch::optim::AdamOptions(learning_rate).weight_decay(weight_decay));

    this->num_epochs = num_epochs;
    this->batch_size = batch_size;
    this->model_name = model_name;
    this->weight_decay = weight_decay;
    this->dropout = dropout;    

    this->num_resblocks = num_resblocks;
    this->num_channels = num_channels;
    this->device = device;
    this->model_name = model_name;
    this->learning_rate = learning_rate;
    this->dataset_path = dataset_path;

    logConfig();
}

SupervisedLearning::~SupervisedLearning()
{
}

void SupervisedLearning::learn()
{


    float min_eval_loss = 100000000.0f;

    for (int epoch = 0; epoch < num_epochs; epoch++)
    {
        logMessage("Epoch: " + std::to_string(epoch + 1), log_file);

        m_ResNetChess->train(true);
        float running_loss = 0.0;
        float batch_count = 0.0;
        auto st = get_time_ms();

        for (int i = 0; i < m_dataset->train_size(); i += batch_size)
        {
            int idx_f = i + batch_size;
            if (idx_f > m_dataset->train_size())
            {
                idx_f = m_dataset->train_size();
            }

            unsigned int b_size = abs(idx_f - i);
            
            torch::Tensor encoded_states = torch::zeros({b_size, 19, 8, 8}, torch::kFloat32); // Initialize the tensor with zeros
            torch::Tensor encoded_actions = torch::zeros({b_size, 8, 8, 73}, torch::kFloat32); // Initialize the tensor with zeros
            torch::Tensor values = torch::zeros({b_size, 1}, torch::kFloat32); // Initialize the tensor with zeros

            for (int j = 0; j < b_size; j++)
            {   
                auto row = m_dataset->getTrain(i + j);

                encoded_states[j] =  row.state.squeeze(0);
                encoded_actions[j] = row.action.squeeze(0);
                values[j] = row.value;
            }


            encoded_states = encoded_states.to(*m_Device);
            encoded_actions = encoded_actions.to(*m_Device);
            values = values.to(*m_Device);

            auto output = m_ResNetChess->forward(encoded_states);

            auto policy_loss = torch::nn::functional::cross_entropy(output.policy, encoded_actions);
            auto value_loss = torch::nn::functional::mse_loss(output.value, values);

            auto loss = policy_loss + value_loss;

            m_Optimizer->zero_grad();
            loss.backward();
            m_Optimizer->step();

            running_loss += loss.cpu().item<float>();
            batch_count += 1.0;
        }
        logMessage(" Train Loss: " + std::to_string(running_loss / batch_count) + " Time: " + std::to_string((float)(get_time_ms() - st) / 1000.0f) + " seconds", log_file);
        logTrain(std::to_string(epoch) + "," + std::to_string(running_loss / batch_count));
        m_dataset->shuffle();
        

        m_ResNetChess->train(false);
        running_loss = 0.0;
        batch_count = 0.0;
        st = get_time_ms();

        for (int i = 0; i < m_dataset->eval_size(); i += batch_size)
        {

            int idx_f = i + batch_size;
            if (idx_f > m_dataset->eval_size())
            {
                idx_f = m_dataset->eval_size();
            }

            unsigned int b_size = abs(idx_f - i);
            
            torch::Tensor encoded_states = torch::zeros({b_size, 19, 8, 8}, torch::kFloat32); // Initialize the tensor with zeros
            torch::Tensor encoded_actions = torch::zeros({b_size, 8, 8, 73}, torch::kFloat32); // Initialize the tensor with zeros
            torch::Tensor values = torch::zeros({b_size, 1}, torch::kFloat32); // Initialize the tensor with zeros

            for (int j = 0; j < b_size; j++)
            {   
                auto row = m_dataset->getEval(i + j);

                encoded_states[j] =  row.state.squeeze(0);
                encoded_actions[j] = row.action.squeeze(0);
                values[j] = row.value;
            }


            encoded_states = encoded_states.to(*m_Device);
            encoded_actions = encoded_actions.to(*m_Device);
            values = values.to(*m_Device);

            auto output = m_ResNetChess->forward(encoded_states);

            auto policy_loss = torch::nn::functional::cross_entropy(output.policy, encoded_actions);
            auto value_loss = torch::nn::functional::mse_loss(output.value, values);

            auto loss = policy_loss + value_loss;

            loss = loss.detach();

            running_loss += loss.cpu().item<float>();
            batch_count += 1.0;
        }
        float eval_loss = running_loss / batch_count;
        logMessage(" Eval Loss: " + std::to_string(running_loss / batch_count) + " Time: " + std::to_string((float)(get_time_ms() - st) / 1000.0f) + " seconds", log_file);
        logEval(std::to_string(epoch) + "," + std::to_string(running_loss / batch_count));
        if (eval_loss < min_eval_loss)
        {
            min_eval_loss = eval_loss;
            save_model(model_path);
            logMessage("Saved best model!!!", log_file);
        }

        logMessage("<------------------------------------------------------------------------------>", log_file);
    }
}

void SupervisedLearning::save_model(std::string path, std::string model_name)
{
    torch::save(m_ResNetChess, path + model_name + ".pt");
    std::cout << "Model saved in: " << path << model_name << ".pt" << std::endl;
}

void SupervisedLearning::load_model(std::string path, std::string model_name)
{
    torch::load(m_ResNetChess, path + model_name + ".pt");
}

void SupervisedLearning::log(std::string message)
{
    logMessage( "[" + getCurrentTimestamp() + "] " + message, model_path + "/log.txt");
    std::cout << "[" << getCurrentTimestamp() << "] " << message << std::endl;
}

void SupervisedLearning::logTrain(std::string message)
{
    logMessage(message, model_path + "/train.csv");
}

void SupervisedLearning::logEval(std::string message)
{
    logMessage(message, model_path + "/eval.csv");
}

void SupervisedLearning::logConfig()
{
    logMessage("{", model_path + "/config.json");
    logMessage("    \"num_epochs\": \"" + std::to_string(num_epochs) + "\",", model_path + "/config.json");
    logMessage("    \"batch_size\": \"" + std::to_string(batch_size) + "\",", model_path + "/config.json");
    logMessage("    \"learning_rate\": \"" + std::to_string(learning_rate) + "\",", model_path + "/config.json");
    logMessage("    \"weight_decay\": \"" + std::to_string(weight_decay) + "\",", model_path + "/config.json");
    logMessage("    \"dropout\": \"" + std::to_string(dropout) + "\",", model_path + "/config.json");
    logMessage("    \"num_resblocks\": \"" + std::to_string(num_resblocks) + "\",", model_path + "/config.json");
    logMessage("    \"num_channels\": \"" + std::to_string(num_channels) + "\",", model_path + "/config.json");
    logMessage("    \"device\": \"" + device + "\",", model_path + "/config.json");
    logMessage("    \"dataset_path\": \"" + dataset_path + "\",", model_path + "/config.json");
    logMessage("}", model_path + "/config.json");
 
}



int main()
{

    std::ifstream config_file("../cfg/config_sl.json");
    if (!config_file.is_open()) {
        std::cerr << "Could not open the config file!" << std::endl;
        return 1;
    }


    // Parse the JSON data
    json config;
    try {
        config_file >> config;
    } catch (const json::parse_error& e) {
        std::cerr << "JSON parse error: " << e.what() << std::endl;
        return 1;
    }

    // Extract values from the JSON object
    int num_epochs = config.value("num_epochs", 0);
    int batch_size = config.value("batch_size", 0);
    double learning_rate = config.value("learning_rate", 0.0);
    double train_split = config.value("train_split", 0.0);
    double weight_decay = config.value("weight_decay", 0.0);
    double dropout = config.value("dropout", 0.0);
    int num_resblocks = config.value("num_resblocks", 0);
    int num_channels = config.value("num_channels", 0);
    std::string model_name = config.value("model_name", "default_model");
    std::string device = config.value("device", "cpu");
    
    std::string dataset_path = "../datasets/games_processed.csv";
    SupervisedLearning sl(
        dataset_path,
        num_epochs,
        learning_rate,
        batch_size,
        train_split,
        weight_decay,
        dropout,
        num_resblocks,
        num_channels,
        device,
        model_name
    );

    sl.learn();

}