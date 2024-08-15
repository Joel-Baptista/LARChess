#pragma once
#include <torch/torch.h>
#include <memory>

struct chess_output {
    torch::Tensor policy;
    torch::Tensor value;
};

struct ResBlock : torch::nn::Module {
    torch::nn::Conv2d conv1{nullptr}, conv2{nullptr};
    torch::nn::BatchNorm2d bn1{nullptr}, bn2{nullptr};

    ResBlock(int64_t num_hidden) {
        conv1 = register_module("conv1", torch::nn::Conv2d(torch::nn::Conv2dOptions(num_hidden, num_hidden, 3).padding(1)));
        bn1 = register_module("bn1", torch::nn::BatchNorm2d(num_hidden));
        conv2 = register_module("conv2", torch::nn::Conv2d(torch::nn::Conv2dOptions(num_hidden, num_hidden, 3).padding(1)));
        bn2 = register_module("bn2", torch::nn::BatchNorm2d(num_hidden));
    }

    torch::Tensor forward(torch::Tensor x) {
        auto residual = x.clone();
        x = torch::relu(bn1(conv1(x)));
        x = bn2(conv2(x));
        x += residual;
        x = torch::relu(x);
        return x;
    }
};

struct ResNetChess : torch::nn::Module {
    torch::nn::Sequential startBlock;
    std::vector<std::shared_ptr<ResBlock>> ptrs_backBone;
    torch::nn::Sequential policyHead;
    torch::nn::Sequential valueHead;

    ResNetChess(int64_t num_resBlocks, int64_t num_hidden, torch::Device device) {
        startBlock = torch::nn::Sequential(
            torch::nn::Conv2d(torch::nn::Conv2dOptions(19, num_hidden, 3).padding(1)),
            torch::nn::BatchNorm2d(num_hidden),
            torch::nn::ReLU()
        );
        register_module("startBlock", startBlock);

        for (int64_t i = 0; i < num_resBlocks; i++) {
            std::shared_ptr<ResBlock> resblock = std::make_shared<ResBlock>(num_hidden);
            ptrs_backBone.push_back(resblock);
            std::string name = "ResBlock" + std::to_string(i);
            register_module(name, resblock);
        }

        policyHead = torch::nn::Sequential(
            torch::nn::Conv2d(torch::nn::Conv2dOptions(num_hidden, 32, 3).padding(1)),
            torch::nn::BatchNorm2d(32),
            torch::nn::ReLU(),
            torch::nn::Flatten(),
            torch::nn::Linear(32 * 8 * 8, 8 * 8 * 73)
        );
        register_module("policyHead", policyHead);

        valueHead = torch::nn::Sequential(
            torch::nn::Conv2d(torch::nn::Conv2dOptions(num_hidden, 3, 3).padding(1)),
            torch::nn::BatchNorm2d(3),
            torch::nn::ReLU(),
            torch::nn::Flatten(),
            torch::nn::Linear(3 * 8 * 8, 1),
            torch::nn::Tanh()
        );
        register_module("valueHead", valueHead);

        to(device);
    }

    chess_output forward(torch::Tensor x) {
        x = startBlock->forward(x);

        int cout = 0;
        for (auto& ptr_resBlock : ptrs_backBone) {
            x = ptr_resBlock->forward(x);
        }

        auto policy = policyHead->forward(x);
        auto value = valueHead->forward(x);

        // policy = policy.view({-1, 8, 8, 73});
        
        return chess_output({policy, value});
    }
};