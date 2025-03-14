#include "../headers/supLearn.h"


#include "../include/json.hpp"

using json = nlohmann::json;

SupervisedLearning::SupervisedLearning(
    std::string& dataset_path,
    int num_epochs, 
    float learning_rate,
    int batch_size,
    float train_split,
    float weight_decay,
    float policy_coef,
    float dropout,
    int num_resblocks,
    int num_channels,
    std::string device,
    std::string model_name,
    std::string precision_type,
    bool hasHeaders = true
    ) 
{
    this->model_path = initLogFiles("../models/slearn");
    log_file = model_path + "/log.txt";

    logMessage("iter,train_loss,eval_loss", model_path + "/train.csv");
    logMessage("iter,train_policy_loss,eval_policy_loss", model_path + "/policy.csv");
    logMessage("iter,train_value_loss,eval_value_loss", model_path + "/value.csv");

    if (precision_type == "float32")
    {
        precision =  torch::kFloat32;
    }
    else if (precision_type == "float16")
    {
        precision = torch::kBFloat16;
    }
    else
    {
        precision = precision;
        logMessage("Precision type {" + precision_type + "} not allowed. Using float32 used by default", log_file);
    }

    m_dataset = std::make_shared<Dataset>(dataset_path, train_split, precision, hasHeaders);
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
    m_ResNetChess->to(precision);
    
    m_Optimizer = std::make_unique<torch::optim::Adam>(m_ResNetChess->parameters(), torch::optim::AdamOptions(learning_rate).weight_decay(weight_decay));

    this->num_epochs = num_epochs;
    this->batch_size = batch_size;
    this->model_name = model_name;
    this->weight_decay = weight_decay;
    this->policy_coef = policy_coef;
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
        float running_policy_loss = 0.0;
        float running_value_loss = 0.0;
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
            
            torch::Tensor encoded_states = torch::zeros({b_size, 19, 8, 8}, precision); // Initialize the tensor with zeros
            torch::Tensor encoded_actions = torch::zeros({b_size, 8, 8, 73}, precision); // Initialize the tensor with zeros
            torch::Tensor values = torch::zeros({b_size, 1}, precision); // Initialize the tensor with zeros
            
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
            
            
            torch::Tensor policy_loss = torch::nn::functional::cross_entropy(output.policy, encoded_actions);
            // torch::Tensor policy_loss = torch::nn::functional::kl_div(policy.log(), encoded_actions);
            // torch::Tensor policy_loss = - torch::mean(encoded_actions * torch::log(policy));
            
            auto value_loss = torch::nn::functional::mse_loss(output.value, values);
            
            auto loss = policy_loss + value_loss;
            
            m_Optimizer->zero_grad();
            loss.backward();
            m_Optimizer->step();

            // double grad_norm = calculate_gradient_norm(m_ResNetChess->parameters());
            // std::cout << std::to_string(grad_norm) << std::endl;

            // torch::nn::utils::clip_grad_norm_(m_ResNetChess->parameters(), 1.0);

            running_loss += loss.cpu().item<float>();
            running_policy_loss += policy_loss.cpu().item<float>();
            running_value_loss += value_loss.cpu().item<float>();
            batch_count += 1.0;
        }
        float train_loss = running_loss / batch_count;
        float train_policy_loss = running_policy_loss / batch_count;
        float train_value_loss = running_value_loss / batch_count;
        logMessage(" Train Loss: " + std::to_string(running_loss / batch_count) + " Time: " + std::to_string((float)(get_time_ms() - st) / 1000.0f) + " seconds (" + std::to_string((float)(get_time_ms() - st) / (batch_count * 1000.0f)) + " ms per batch)", log_file);
        // logTrain(std::to_string(epoch) + "," + std::to_string(running_loss / batch_count));
        m_dataset->shuffle();
        

        m_ResNetChess->train(false);
        running_loss = 0.0;
        running_policy_loss = 0.0;
        running_value_loss = 0.0;
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
            
            torch::Tensor encoded_states = torch::zeros({b_size, 19, 8, 8}, precision); // Initialize the tensor with zeros
            torch::Tensor encoded_actions = torch::zeros({b_size, 8, 8, 73}, precision); // Initialize the tensor with zeros
            torch::Tensor values = torch::zeros({b_size, 1}, precision); // Initialize the tensor with zeros

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

            torch::Tensor policy_loss = torch::nn::functional::cross_entropy(output.policy, encoded_actions);
            // torch::Tensor policy_loss = torch::nn::functional::kl_div(policy.log(), encoded_actions);
            // torch::Tensor policy_loss = - torch::mean(encoded_actions * torch::log(policy));

            auto value_loss = torch::nn::functional::mse_loss(output.value, values);

            auto loss = policy_loss + value_loss;
            loss = loss.detach();

            running_loss += loss.cpu().item<float>();
            running_policy_loss += policy_loss.cpu().item<float>();
            running_value_loss += value_loss.cpu().item<float>();
            batch_count += 1.0;
        }
        float eval_loss = running_loss / batch_count;
        float eval_policy_loss = running_policy_loss / batch_count;
        float eval_value_loss = running_value_loss / batch_count;
        logMessage(" Eval Loss: " + std::to_string(running_loss / batch_count) + " Time: " + std::to_string((float)(get_time_ms() - st) / 1000.0f) + " seconds", log_file);
        logTrain(std::to_string(epoch) + "," + std::to_string(train_loss) + "," + std::to_string(eval_loss));
        logMessage(std::to_string(epoch) + "," + std::to_string(train_policy_loss) + "," + std::to_string(eval_policy_loss), model_path + "/policy.csv");
        logMessage(std::to_string(epoch) + "," + std::to_string(train_value_loss) + "," + std::to_string(eval_value_loss), model_path + "/value.csv");
        if (eval_loss < min_eval_loss)
        {
            min_eval_loss = eval_loss;
            save_model(model_path);
            logMessage("Saved best model!!!", log_file);
        }

        logMessage("<------------------------------------------------------------------------------>", log_file);
    }
}

void SupervisedLearning::save_model(std::string path)
{
    torch::save(m_ResNetChess, path + "/model.pt");
    std::cout << "Model saved in: " << path << "/model.pt" << std::endl;
}

void SupervisedLearning::load_model(std::string path)
{
    torch::load(m_ResNetChess, path + "/model.pt");
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
    logMessage("    \"policy_coef\": \"" + std::to_string(policy_coef) + "\",", model_path + "/config.json");
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
    double policy_coef = config.value("policy_coef", 0.0);
    double dropout = config.value("dropout", 0.0);
    int num_resblocks = config.value("num_resblocks", 0);
    int num_channels = config.value("num_channels", 0);
    std::string model_name = config.value("model_name", "default_model");
    std::string precision_type = config.value("precision", "float32");
    std::string device = config.value("device", "cpu");
    
    std::string dataset_path = "../datasets/dataset.csv";
    SupervisedLearning sl(
        dataset_path,
        num_epochs,
        learning_rate,
        batch_size,
        train_split,
        weight_decay,
        policy_coef,
        dropout,
        num_resblocks,
        num_channels,
        device,
        model_name,
        precision_type
    );

    sl.learn();

}
