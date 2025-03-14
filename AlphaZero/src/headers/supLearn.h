#pragma once
#include "dataset.h"

class SupervisedLearning {

    public:
        SupervisedLearning(
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
                bool hasHeaders
            );
        ~SupervisedLearning();

        void learn();
        void save_model(std::string path);
        void load_model(std::string path);
        void save_model() { save_model(""); }
        void load_model() { load_model(""); }
        void log(std::string message);
        void logTrain(std::string message);
        void logEval(std::string message);
        void logConfig();

    private:
        std::shared_ptr<Dataset> m_dataset;

        std::shared_ptr<ResNetChess> m_ResNetChess;
        std::unique_ptr<torch::optim::Adam> m_Optimizer;
        std::shared_ptr<torch::Device> m_Device;

        std::string log_file;

        int num_epochs;
        int batch_size;
        float train_split;
        std::string model_name;
        std::string model_path;

        std::string dataset_path; 
        float learning_rate;
        float weight_decay;
        float policy_coef;
        float dropout;
        int num_resblocks;
        int num_channels;
        std::string device;
        c10::ScalarType precision;
        bool hasHeaders;
        
};