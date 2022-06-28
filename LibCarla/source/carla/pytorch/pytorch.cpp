// Copyright (c) 2022 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB).
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#define _GLIBCXX_USE_CXX11_ABI 0

#include "pytorch.h"

#include <torch/torch.h>
#include <torch/script.h>
#include <torchscatter/scatter.h>
#include <torchcluster/cluster.h>
// #include <torchsparse/sparse.h>
#include <string>
#include <istream>
#include <vector>
 
namespace carla {
namespace learning {

  void test_learning()
  {
    std::cout << "cuda version " << cuda_version() << std::endl;
    std::cout << "cuda version " << scatter::cuda_version() << std::endl;
    torch::Tensor tensor = torch::eye(3);
    std::cout << tensor << std::endl;
  }

  // holds the neural network
  struct NeuralModelImpl
  {
    torch::jit::script::Module module;
  };

  NeuralModel::NeuralModel() {
    Model = std::make_unique<NeuralModelImpl>();
  }
  void NeuralModel::LoadModel(char* filename) {
    std::string filename_str(filename);
    std::cout << "loading " << filename_str << std::endl;
    // Model->module = torch::jit::load(filename_str, c10::DeviceType::CUDA);
    Model->module = torch::jit::load(filename_str);
    std::cout << "loaded " << filename_str <<  std::endl;
  }

  void NeuralModel::SetIputs(Inputs input) {
    _input = input;
  }

  void NeuralModel::Forward() {
    std::vector<torch::jit::IValue> TorchInputs;

    // Loop over all vehicle wheels
    for (int i = 0; i < 4; i++) {
        // Prepare the tuple input for this wheel
        int num_particles = 1000;
        std::vector<torch::jit::IValue> tuple;
        tuple.push_back(torch::empty({num_particles, 3}, torch::kFloat32)); //pos particles
        tuple.push_back(torch::empty({num_particles, 3}, torch::kFloat32)); //vel particles
        tuple.push_back(torch::empty({3}, torch::kFloat32)); //wheel pos
        tuple.push_back(torch::empty({4}, torch::kFloat32)); //wheel rot (quaternion)
        tuple.push_back(torch::empty({3}, torch::kFloat32)); //wheel velocity
        tuple.push_back(torch::empty({3}, torch::kFloat32)); //wheel ang-velocity

        // Add this wheel's tuple to NN model inputs
        TorchInputs.push_back(torch::ivalue::Tuple::create(tuple));
    }

    // Load vehicle data (1 tensor)
    auto drv_inputs = torch::tensor({(float)0, (float)0, (float)0}, torch::kFloat32); //steer, throtle, brake

    // Add the vehicle data to NN model inputs
    TorchInputs.push_back(drv_inputs);

    torch::jit::IValue Output = Model->module.forward(TorchInputs);
  }

  Outputs NeuralModel::GetOutputs() {
    return _output;
  }

  NeuralModel::~NeuralModel() {}

}
}
