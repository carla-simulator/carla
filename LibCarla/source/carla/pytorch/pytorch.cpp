// Copyright (c) 2024 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB).
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#define _GLIBCXX_USE_CXX11_ABI 0

#include "pytorch.h"

#include <torch/torch.h>
#include <torch/script.h>
#include <torchscatter/scatter.h>
#include <torchcluster/cluster.h>
#include <torch/csrc/jit/passes/tensorexpr_fuser.h>
#include <c10/cuda/CUDACachingAllocator.h>
#include <string>
#include <vector>
#include <ostream>
#include <sstream>

void add_mark(const std::string &text) {

}

namespace carla {
namespace learning {

  void test_learning()
  {
    std::ostringstream ss;
    std::cout << "cuda version " << cluster::cuda_version() << std::endl;
    std::cout << "cuda version " << scatter::cuda_version() << std::endl;
    // torch::Tensor tensor = torch::eye(3);
    // std::cout << tensor << std::endl;
  }

  torch::jit::IValue GetWheelTensorInputs(WheelInput& wheel) {
    at::Tensor particles_position_tensor = 
        torch::from_blob(wheel.particles_positions, 
            {wheel.num_particles, 3}, torch::kFloat32);

    at::Tensor particles_velocity_tensor = 
        torch::from_blob(wheel.particles_velocities, 
            {wheel.num_particles, 3}, torch::kFloat32);

    at::Tensor wheel_positions_tensor = 
        torch::from_blob(wheel.wheel_positions, 
            {3}, torch::kFloat32);

    at::Tensor wheel_oritentation_tensor = 
        torch::from_blob(wheel.wheel_oritentation, 
            {4}, torch::kFloat32);

    at::Tensor wheel_linear_velocity_tensor = 
        torch::from_blob(wheel.wheel_linear_velocity, 
            {3}, torch::kFloat32);

    at::Tensor wheel_angular_velocity_tensor = 
        torch::from_blob(wheel.wheel_angular_velocity, 
            {3}, torch::kFloat32);

    std::vector<torch::jit::IValue> Tuple 
        {particles_position_tensor, particles_velocity_tensor, wheel_positions_tensor, 
         wheel_oritentation_tensor, wheel_linear_velocity_tensor, wheel_angular_velocity_tensor};
    return torch::ivalue::Tuple::create(Tuple);
  }

  WheelOutput GetWheelTensorOutput(
      const at::Tensor &particle_forces, 
      const at::Tensor &wheel_forces ) {
    WheelOutput result;
    const float* wheel_forces_data = wheel_forces.data_ptr<float>();
    result.wheel_forces_x = wheel_forces_data[0];
    result.wheel_forces_y = wheel_forces_data[1];
    result.wheel_forces_z = wheel_forces_data[2];
    result.wheel_torque_x = wheel_forces_data[3];
    result.wheel_torque_y = wheel_forces_data[4];
    result.wheel_torque_z = wheel_forces_data[5];
    const float* particle_forces_data = particle_forces.data_ptr<float>();
    int num_dimensions = 3;
    int num_particles = particle_forces.sizes()[0];
    result._particle_forces.reserve(num_particles*num_dimensions);
    for (int i = 0; i < num_particles; i++) {
      result._particle_forces.emplace_back(
          particle_forces_data[i*num_dimensions + 0]);
      result._particle_forces.emplace_back(
          particle_forces_data[i*num_dimensions + 1]);
      result._particle_forces.emplace_back(
          particle_forces_data[i*num_dimensions + 2]);
    }
    return result;
  }

  WheelOutput GetWheelTensorOutputDynamic(
      const at::Tensor &particle_forces, 
      const at::Tensor &wheel_forces) {
    WheelOutput result;
    const float* wheel_forces_data = wheel_forces.data_ptr<float>();
    result.wheel_forces_x = wheel_forces_data[0];
    result.wheel_forces_y = wheel_forces_data[1];
    result.wheel_forces_z = wheel_forces_data[2];
    const float* particle_forces_data = particle_forces.data_ptr<float>();
    int num_dimensions = 3;
    int num_particles = particle_forces.sizes()[0];
    result._particle_forces.reserve(num_particles*num_dimensions);
    for (int i = 0; i < num_particles; i++) {
      result._particle_forces.emplace_back(
          particle_forces_data[i*num_dimensions + 0]);
      result._particle_forces.emplace_back(
          particle_forces_data[i*num_dimensions + 1]);
      result._particle_forces.emplace_back(
          particle_forces_data[i*num_dimensions + 2]);
    }
    return result;
  }

  // holds the neural network
  struct NeuralModelImpl
  {
    NeuralModelImpl(){}
    torch::jit::script::Module module;
    ~NeuralModelImpl(){}
    std::vector<at::Tensor> particles_position_tensors;
    std::vector<at::Tensor> particles_velocity_tensors;
    torch::jit::IValue GetWheelTensorInputsCUDA(WheelInput& wheel, int wheel_idx);
  };
  torch::jit::IValue NeuralModelImpl::GetWheelTensorInputsCUDA(WheelInput& wheel, int wheel_idx)
  {
    at::Tensor particles_position_tensor = 
        torch::from_blob(wheel.particles_positions, 
            {wheel.num_particles, 3}, torch::kFloat32);

    at::Tensor particles_velocity_tensor = 
        torch::from_blob(wheel.particles_velocities, 
            {wheel.num_particles, 3}, torch::kFloat32);

    at::Tensor wheel_positions_tensor = 
        torch::from_blob(wheel.wheel_positions, 
            {3}, torch::kFloat32);

    at::Tensor wheel_oritentation_tensor = 
        torch::from_blob(wheel.wheel_oritentation, 
            {4}, torch::kFloat32);

    at::Tensor wheel_linear_velocity_tensor = 
        torch::from_blob(wheel.wheel_linear_velocity, 
            {3}, torch::kFloat32);

    at::Tensor wheel_angular_velocity_tensor = 
        torch::from_blob(wheel.wheel_angular_velocity, 
            {3}, torch::kFloat32);

    std::vector<torch::jit::IValue> Tuple 
        {particles_position_tensor.cuda(), particles_velocity_tensor.cuda(), wheel_positions_tensor.cuda(), 
         wheel_oritentation_tensor.cuda(), wheel_linear_velocity_tensor.cuda(), wheel_angular_velocity_tensor.cuda(),
         wheel.num_particles};
    return torch::ivalue::Tuple::create(Tuple);
  }

  NeuralModel::NeuralModel() {
    Model = std::make_unique<NeuralModelImpl>();
  }
  void NeuralModel::LoadModel(char* filename, int device) {
    torch::jit::setTensorExprFuserEnabled(false);
    std::string filename_str(filename);
    std::cout << "loading " << filename_str << std::endl;
    try {
      Model->module = torch::jit::load(filename_str);
      std::string cuda_str = "cuda:" + std::to_string(device);
      // std::cout << "Using CUDA device " << cuda_str << std::endl;
      // Model->module.to(at::Device(cuda_str));
    } catch (const c10::Error& e) {
      std::cout << "Error loading model: " << e.msg() << std::endl;
    }
    std::cout << "loaded " << filename_str <<  std::endl;
  }

  void NeuralModel::SetInputs(Inputs input) {
    _input = input;
  }


  void NeuralModel::Forward() {
    std::vector<torch::jit::IValue> TorchInputs;
    TorchInputs.push_back(GetWheelTensorInputs(_input.wheel0));
    TorchInputs.push_back(GetWheelTensorInputs(_input.wheel1));
    TorchInputs.push_back(GetWheelTensorInputs(_input.wheel2));
    TorchInputs.push_back(GetWheelTensorInputs(_input.wheel3));
    auto drv_inputs = torch::tensor(
        {_input.steering, _input.throttle, _input.braking}, torch::kFloat32); //steer, throtle, brake
    TorchInputs.push_back(drv_inputs);
    if (_input.terrain_type >= 0) {
      TorchInputs.push_back(_input.terrain_type);
    }
    TorchInputs.push_back(_input.verbose);

    torch::jit::IValue Output;
    try {
      Output = Model->module.forward(TorchInputs);
    } catch (const c10::Error& e) {
      std::cout << "Error running model: " << e.msg() << std::endl;
    }

    std::vector<torch::jit::IValue> Tensors =  Output.toTuple()->elements();
    _output.wheel0 = GetWheelTensorOutput(
        Tensors[0].toTensor().cpu(), Tensors[4].toTensor().cpu() );
    _output.wheel1 = GetWheelTensorOutput(
        Tensors[1].toTensor().cpu(), Tensors[5].toTensor().cpu() );
    _output.wheel2 = GetWheelTensorOutput(
        Tensors[2].toTensor().cpu(), Tensors[6].toTensor().cpu() );
    _output.wheel3 = GetWheelTensorOutput(
        Tensors[3].toTensor().cpu(), Tensors[7].toTensor().cpu() );

  }
  void NeuralModel::ForwardDynamic() {
    {

      std::vector<torch::jit::IValue> TorchInputs;
      TorchInputs.push_back(GetWheelTensorInputs(_input.wheel0));
      TorchInputs.push_back(GetWheelTensorInputs(_input.wheel1));
      TorchInputs.push_back(GetWheelTensorInputs(_input.wheel2));
      TorchInputs.push_back(GetWheelTensorInputs(_input.wheel3));
      auto drv_inputs = torch::tensor(
          {_input.steering, _input.throttle, _input.braking}, torch::kFloat32); //steer, throtle, brake
      TorchInputs.push_back(drv_inputs);
      if (_input.terrain_type >= 0) {
        TorchInputs.push_back(_input.terrain_type);
      }
      TorchInputs.push_back(_input.verbose);

      torch::jit::IValue Output;
      try {
        Output = Model->module.forward(TorchInputs);
      } catch (const c10::Error& e) {
        std::cout << "Error running model: " << e.msg() << std::endl;
      }

      std::vector<torch::jit::IValue> Tensors =  Output.toTuple()->elements();
      _output.wheel0 = GetWheelTensorOutputDynamic(
          Tensors[0].toTensor().cpu(), Tensors[4].toTensor().cpu());
      _output.wheel1 = GetWheelTensorOutputDynamic(
          Tensors[1].toTensor().cpu(), Tensors[5].toTensor().cpu());
      _output.wheel2 = GetWheelTensorOutputDynamic(
          Tensors[2].toTensor().cpu(), Tensors[6].toTensor().cpu());
      _output.wheel3 = GetWheelTensorOutputDynamic(
          Tensors[3].toTensor().cpu(), Tensors[7].toTensor().cpu());

    }

    c10::cuda::CUDACachingAllocator::emptyCache();

  }

  void NeuralModel::ForwardCUDATensors()
  {
    std::vector<torch::jit::IValue> TorchInputs;
    TorchInputs.push_back(Model->GetWheelTensorInputsCUDA(_input.wheel0, 0));
    TorchInputs.push_back(Model->GetWheelTensorInputsCUDA(_input.wheel1, 1));
    TorchInputs.push_back(Model->GetWheelTensorInputsCUDA(_input.wheel2, 2));
    TorchInputs.push_back(Model->GetWheelTensorInputsCUDA(_input.wheel3, 3));
    auto drv_inputs = torch::tensor(
        {_input.steering, _input.throttle, _input.braking}, torch::kFloat32); //steer, throtle, brake
    TorchInputs.push_back(drv_inputs.cuda());
    if (_input.terrain_type >= 0) {
      TorchInputs.push_back(_input.terrain_type);
    }
    TorchInputs.push_back(_input.verbose);

    torch::jit::IValue Output;
    try {
      Output = Model->module.forward(TorchInputs);
    } catch (const c10::Error& e) {
      std::cout << "Error running model: " << e.msg() << std::endl;
    }

    std::vector<torch::jit::IValue> Tensors =  Output.toTuple()->elements();
    _output.wheel0 = GetWheelTensorOutput(
        Tensors[0].toTensor().cpu(), Tensors[4].toTensor().cpu() );
    _output.wheel1 = GetWheelTensorOutput(
        Tensors[1].toTensor().cpu(), Tensors[5].toTensor().cpu() );
    _output.wheel2 = GetWheelTensorOutput(
        Tensors[2].toTensor().cpu(), Tensors[6].toTensor().cpu() );
    _output.wheel3 = GetWheelTensorOutput(
        Tensors[3].toTensor().cpu(), Tensors[7].toTensor().cpu() );
  }
  
  Outputs& NeuralModel::GetOutputs() {
    return _output;
  }

  NeuralModel::~NeuralModel() {}

}
}
