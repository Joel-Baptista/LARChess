#pragma once

#include "game.h"

torch::Tensor xtensor_to_torch(xt::xtensor<float, 4> xtensor);
torch::Tensor xtensor_to_torch(xt::xtensor<float, 2> xtensor);
xt::xtensor<float, 3> torch_to_tensor(torch::Tensor torch_tensor);