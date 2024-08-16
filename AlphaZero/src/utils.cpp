#include "utils.h"


torch::Tensor xtensor_to_torch(xt::xtensor<float, 4> xtensor)
{
    // Obtain the shape of the xtensor array
    std::vector<int64_t> shape(xtensor.shape().begin(), xtensor.shape().end());

    // Create a libtorch tensor from the raw data pointer of xtensor
    torch::Tensor torch_tensor = torch::from_blob(xtensor.data(), shape, torch::kFloat32);

    return torch_tensor;
}

torch::Tensor xtensor_to_torch(xt::xtensor<float, 2> xtensor)
{
    // Obtain the shape of the xtensor array
    std::vector<int64_t> shape(xtensor.shape().begin(), xtensor.shape().end());

    // Create a libtorch tensor from the raw data pointer of xtensor
    torch::Tensor torch_tensor = torch::from_blob(xtensor.data(), shape, torch::kFloat32);

    return torch_tensor;
}


xt::xtensor<float, 3> torch_to_tensor(torch::Tensor torch_tensor)
{
    auto torch_shape = torch_tensor.sizes().vec();
    // Create an xtensor array that uses the raw data from the torch tensor
    xt::xtensor<float, 3> xarray = xt::adapt(
        torch_tensor.data_ptr<float>(),  // Pointer to the data
        torch_tensor.numel(),            // Number of elements
        xt::no_ownership(),              // Memory management (no ownership)
        torch_shape                      // Shape of the tensor
    ); 

    return xarray;
}