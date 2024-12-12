#pragma once
#include <torch/torch.h>

struct ResBlockImpl : torch::nn::Module {
    torch::nn::Conv2d conv1{nullptr}, conv2{nullptr};
    torch::nn::BatchNorm2d bn1{nullptr}, bn2{nullptr};

    ResBlockImpl(int64_t num_hidden) {
        conv1 = register_module("conv1", torch::nn::Conv2d(torch::nn::Conv2dOptions(num_hidden, num_hidden, 3).padding(1).bias(false)));
        bn1 = register_module("bn1", torch::nn::BatchNorm2d(num_hidden));
        conv2 = register_module("conv2", torch::nn::Conv2d(torch::nn::Conv2dOptions(num_hidden, num_hidden, 3).padding(1).bias(false)));
        bn2 = register_module("bn2", torch::nn::BatchNorm2d(num_hidden));
    }

    torch::Tensor forward(torch::Tensor x) {
        auto residual = x.clone();
        x = torch::relu(bn1(conv1(x)));
        x = bn2(conv2(x));
        x += residual;
        return torch::relu(x);
    }
};
TORCH_MODULE(ResBlock);

struct ResNetChessImpl : torch::nn::Module {
    torch::nn::Sequential startBlock;
    torch::nn::ModuleList ptrs_backBone;
    torch::nn::Sequential policyHead;
    torch::nn::Sequential valueHead;

    ResNetChessImpl(int64_t num_resBlocks, int64_t num_hidden, float dropout) {
        startBlock = torch::nn::Sequential(
            torch::nn::Conv2d(torch::nn::Conv2dOptions(19, num_hidden, 3).padding(1).bias(false)),
            torch::nn::BatchNorm2d(num_hidden),
            torch::nn::ReLU()
        );
        register_module("startBlock", startBlock);

        for (int64_t i = 0; i < num_resBlocks; i++) {
            ptrs_backBone->push_back(ResBlock(num_hidden));
        }
        register_module("ptrs_backBone", ptrs_backBone);

        policyHead = torch::nn::Sequential(
            torch::nn::Conv2d(torch::nn::Conv2dOptions(num_hidden, num_hidden, 3).padding(1).bias(false)),
            torch::nn::BatchNorm2d(num_hidden),
            torch::nn::ReLU(),
            torch::nn::Flatten(),
            torch::nn::Dropout(dropout),
            torch::nn::Linear(num_hidden * 8 * 8, 8 * 8 * 73)
        );
        register_module("policyHead", policyHead);

        valueHead = torch::nn::Sequential(
            torch::nn::Conv2d(torch::nn::Conv2dOptions(num_hidden, num_hidden, 3).padding(1).bias(false)),
            torch::nn::BatchNorm2d(num_hidden),
            torch::nn::ReLU(),
            torch::nn::Flatten(),
            torch::nn::Dropout(dropout),
            torch::nn::Linear(num_hidden * 8 * 8, 1),
            torch::nn::Tanh()
        );
        register_module("valueHead", valueHead);
    }

    std::tuple<torch::Tensor, torch::Tensor> forward(torch::Tensor x) {
        x = startBlock->forward(x);

        for (auto& resBlock : *ptrs_backBone) {
            x = resBlock->as<ResBlock>()->forward(x);
        }

        auto policy = policyHead->forward(x);
        auto value = valueHead->forward(x);

        policy = policy.view({-1, 8, 8, 73});
        return std::make_tuple(policy, value);
    }
};
TORCH_MODULE(ResNetChess);
