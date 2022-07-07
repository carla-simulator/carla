// Copyright (c) 2022 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB).
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#define _GLIBCXX_USE_CXX11_ABI 0

#include "pytorch.h"

#include <torch/torch.h>
#include <torch/script.h>
#include <torchscatter/scatter.h>
#include <torchcluster/cluster.h>
#include <iostream>
#include <string>
#include <istream>
#include <vector>
#include <fstream>
#include <sstream>


static at::Tensor read_tensor(std::string filename, int dim = 1) {
  std::ifstream ifs;
  ifs.open (filename, std::ifstream::in);
  std::string line;
  int num_lines = 0;
  while(true) {
    std::getline(ifs, line);
    if (ifs.eof()) {
      break;
    }
    num_lines++;
  }
  ifs.clear();
  ifs.seekg(0);
  std::vector<float> data;
  while(true) {
    std::getline(ifs, line);
    if (ifs.eof()) {
      break;
    }
    std::istringstream iss(line);
    float x, y, z;
    // if (!(iss >> x >> y >> z)) { break; } // error
    // std::cout << x << " " << y << " " << z << " " << std::endl;
    for(int i = 0; i < dim; i++) {
      float value;
      iss >> value;
      // std::cout << value << " ";
      data.emplace_back(value);
    }
    // std::cout << std::endl;
    // process pair (a,b)
  }
  // std::cout << num_lines<< std::endl;
  at::Tensor Tensor = torch::empty({num_lines, dim}, torch::kFloat32);
  if(dim == 1){
    return torch::from_blob(data.data(), {num_lines}, torch::kFloat32).clone();
  }
  else{
    return torch::from_blob(data.data(), {num_lines, dim}, torch::kFloat32).clone();
  }
  // return torch::empty({1,1}, torch::kFloat32);
}

static void print_tensor_size(const at::Tensor& Tensor) {
  for(int s : Tensor.sizes()){ 
    std::cout << s << " ";
  }
  std::cout << std::endl;
}

static void print_tensor_sizes(const std::vector<at::Tensor> & Tensors) {
  for(const auto & Tensor : Tensors) {
    print_tensor_size(Tensor);
  }
}
namespace carla {
namespace learning {

  void test_learning()
  {
    std::cout << "cuda version " << cuda_version() << std::endl;
    std::cout << "cuda version " << scatter::cuda_version() << std::endl;
    torch::Tensor tensor = torch::eye(3);
    std::cout << tensor << std::endl;
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
  void WheelOutput::SetParticleFloatData(std::vector<float> &data)
  {
    _particle_forces = data;
  }
  WheelOutput GetWheelTensorOutput(
      const at::Tensor &particle_forces, 
      const at::Tensor &wheel_forces) {
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

  void NeuralModel::SetInputs(Inputs input) {
    _input = input;
  }

  void NeuralModel::Forward_Test() {
  at::Tensor soil_pos_0 = read_tensor("/media/axel/DATAEXT/Downloads/inputsforaxel/soilpos_0.txt", 3);
  at::Tensor soil_pos_1 = read_tensor("/media/axel/DATAEXT/Downloads/inputsforaxel/soilpos_1.txt", 3);
  at::Tensor soil_pos_2 = read_tensor("/media/axel/DATAEXT/Downloads/inputsforaxel/soilpos_2.txt", 3);
  at::Tensor soil_pos_3 = read_tensor("/media/axel/DATAEXT/Downloads/inputsforaxel/soilpos_3.txt", 3);

  at::Tensor soil_vel_0 = read_tensor("/media/axel/DATAEXT/Downloads/inputsforaxel/soilvel_0.txt", 3);
  at::Tensor soil_vel_1 = read_tensor("/media/axel/DATAEXT/Downloads/inputsforaxel/soilvel_1.txt", 3);
  at::Tensor soil_vel_2 = read_tensor("/media/axel/DATAEXT/Downloads/inputsforaxel/soilvel_2.txt", 3);
  at::Tensor soil_vel_3 = read_tensor("/media/axel/DATAEXT/Downloads/inputsforaxel/soilvel_3.txt", 3);
  
  at::Tensor wheel_linvel_0 = read_tensor("/media/axel/DATAEXT/Downloads/inputsforaxel/wheellinvel_0.txt");
  at::Tensor wheel_linvel_1 = read_tensor("/media/axel/DATAEXT/Downloads/inputsforaxel/wheellinvel_1.txt");
  at::Tensor wheel_linvel_2 = read_tensor("/media/axel/DATAEXT/Downloads/inputsforaxel/wheellinvel_2.txt");
  at::Tensor wheel_linvel_3 = read_tensor("/media/axel/DATAEXT/Downloads/inputsforaxel/wheellinvel_3.txt");

  at::Tensor wheel_angvel_0 = read_tensor("/media/axel/DATAEXT/Downloads/inputsforaxel/wheelangvel_0.txt");
  at::Tensor wheel_angvel_1 = read_tensor("/media/axel/DATAEXT/Downloads/inputsforaxel/wheelangvel_1.txt");
  at::Tensor wheel_angvel_2 = read_tensor("/media/axel/DATAEXT/Downloads/inputsforaxel/wheelangvel_2.txt");
  at::Tensor wheel_angvel_3 = read_tensor("/media/axel/DATAEXT/Downloads/inputsforaxel/wheelangvel_3.txt");
  
  at::Tensor wheel_orient_0 = read_tensor("/media/axel/DATAEXT/Downloads/inputsforaxel/wheelorient_0.txt");
  at::Tensor wheel_orient_1 = read_tensor("/media/axel/DATAEXT/Downloads/inputsforaxel/wheelorient_1.txt");
  at::Tensor wheel_orient_2 = read_tensor("/media/axel/DATAEXT/Downloads/inputsforaxel/wheelorient_2.txt");
  at::Tensor wheel_orient_3 = read_tensor("/media/axel/DATAEXT/Downloads/inputsforaxel/wheelorient_3.txt");

  at::Tensor wheel_pos_0 = read_tensor("/media/axel/DATAEXT/Downloads/inputsforaxel/wheelpos_0.txt");
  at::Tensor wheel_pos_1 = read_tensor("/media/axel/DATAEXT/Downloads/inputsforaxel/wheelpos_1.txt");
  at::Tensor wheel_pos_2 = read_tensor("/media/axel/DATAEXT/Downloads/inputsforaxel/wheelpos_2.txt");
  at::Tensor wheel_pos_3 = read_tensor("/media/axel/DATAEXT/Downloads/inputsforaxel/wheelpos_3.txt");

  // print_tensor_sizes({soil_pos_0, soil_pos_1, soil_pos_2, soil_pos_3,
  //                     soil_vel_0, soil_vel_1, soil_vel_2, soil_vel_3,
  //                     wheel_linvel_0, wheel_linvel_1, wheel_linvel_2, wheel_linvel_3,
  //                     wheel_angvel_0, wheel_angvel_1, wheel_angvel_2, wheel_angvel_3,
  //                     wheel_orient_0, wheel_orient_1, wheel_orient_2, wheel_orient_3,
  //                     wheel_pos_0, wheel_pos_1, wheel_pos_2, wheel_pos_3});
    std::vector<torch::jit::IValue> TorchInputs;

    // // Loop over all vehicle wheels
    // for (int i = 0; i < 4; i++) {
    //     // Prepare the tuple input for this wheel
    //     int num_particles = 1000;
    //     std::vector<torch::jit::IValue> tuple;
    //     tuple.push_back(torch::empty({num_particles, 3}, torch::kFloat32)); //pos particles
    //     tuple.push_back(torch::empty({num_particles, 3}, torch::kFloat32)); //vel particles
    //     tuple.push_back(torch::empty({3}, torch::kFloat32)); //wheel pos
    //     tuple.push_back(torch::empty({4}, torch::kFloat32)); //wheel rot (quaternion)
    //     tuple.push_back(torch::empty({3}, torch::kFloat32)); //wheel velocity
    //     tuple.push_back(torch::empty({3}, torch::kFloat32)); //wheel ang-velocity

    //     // Add this wheel's tuple to NN model inputs
    //     TorchInputs.push_back(torch::ivalue::Tuple::create(tuple));
    // }

    std::vector<torch::jit::IValue> tuple0 {soil_pos_0, soil_vel_0, wheel_pos_0, wheel_orient_0, wheel_linvel_0, wheel_angvel_0};
    std::vector<torch::jit::IValue> tuple1 {soil_pos_1, soil_vel_1, wheel_pos_1, wheel_orient_1, wheel_linvel_1, wheel_angvel_1};
    std::vector<torch::jit::IValue> tuple2 {soil_pos_2, soil_vel_2, wheel_pos_2, wheel_orient_2, wheel_linvel_2, wheel_angvel_2};
    std::vector<torch::jit::IValue> tuple3 {soil_pos_3, soil_vel_3, wheel_pos_3, wheel_orient_3, wheel_linvel_3, wheel_angvel_3};

    TorchInputs.push_back(torch::ivalue::Tuple::create(tuple0));
    TorchInputs.push_back(torch::ivalue::Tuple::create(tuple1));
    TorchInputs.push_back(torch::ivalue::Tuple::create(tuple2));
    TorchInputs.push_back(torch::ivalue::Tuple::create(tuple3));
    // Load vehicle data (1 tensor)
    auto drv_inputs = torch::tensor({(float)0, (float)0, (float)0}, torch::kFloat32); //steer, throtle, brake

    // Add the vehicle data to NN model inputs
    TorchInputs.push_back(drv_inputs);

    torch::jit::IValue Output = Model->module.forward(TorchInputs);

    TorchInputs.push_back(GetWheelTensorInputs(_input.wheel0));
  }

  void NeuralModel::Forward() {
    std::vector<torch::jit::IValue> TorchInputs;
    TorchInputs.push_back(GetWheelTensorInputs(_input.wheel0));
    TorchInputs.push_back(GetWheelTensorInputs(_input.wheel1));
    TorchInputs.push_back(GetWheelTensorInputs(_input.wheel2));
    TorchInputs.push_back(GetWheelTensorInputs(_input.wheel3));
    auto drv_inputs = torch::tensor({(float)0, (float)0, (float)0}, torch::kFloat32); //steer, throtle, brake
    TorchInputs.push_back(drv_inputs);
    TorchInputs.push_back(_input.verbose);

    torch::jit::IValue Output = Model->module.forward(TorchInputs);

    std::vector<torch::jit::IValue> Tensors =  Output.toTuple()->elements();
    _output.wheel0 = GetWheelTensorOutput(
        Tensors[0].toTensor().cpu(), Tensors[4].toTensor().cpu());
    _output.wheel1 = GetWheelTensorOutput(
        Tensors[1].toTensor().cpu(), Tensors[5].toTensor().cpu());
    _output.wheel2 = GetWheelTensorOutput(
        Tensors[2].toTensor().cpu(), Tensors[6].toTensor().cpu());
    _output.wheel3 = GetWheelTensorOutput(
        Tensors[3].toTensor().cpu(), Tensors[7].toTensor().cpu());
  }

  Outputs& NeuralModel::GetOutputs() {
    return _output;
  }

  NeuralModel::~NeuralModel() {}

}
}
