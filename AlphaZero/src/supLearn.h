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
                int num_resblocks,
                int num_channels,
                std::string device, 
                std::string model_name,
                bool hasHeaders
            );
        ~SupervisedLearning();

        void learn();
        void save_model(std::string path, std::string model_name);
        void load_model(std::string path, std::string model_name);
        void save_model(std::string model_name) { save_model("", model_name); }
        void load_model(std::string model_name) { load_model("", model_name); }

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
        
};