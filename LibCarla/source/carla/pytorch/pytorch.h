// Copyright (c) 2024 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB).
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once
#define _GLIBCXX_USE_CXX11_ABI 0

#include <memory>
#include <vector>

namespace carla {
namespace learning {

  void test_learning();

  struct NeuralModelImpl;

  struct WheelInput {
  public:
    int num_particles = 0;
    float* particles_positions;
    float* particles_velocities;
    float* wheel_positions;
    float* wheel_oritentation;
    float* wheel_linear_velocity;
    float* wheel_angular_velocity;
  };

  struct Inputs {
  public:
    WheelInput wheel0;
    WheelInput wheel1;
    WheelInput wheel2;
    WheelInput wheel3;
    float steering = 0;
    float throttle = 0;
    float braking = 0;
    int terrain_type = 0;
    bool verbose = false;
  };
 
  struct WheelOutput {
  public:
    float wheel_forces_x = 0;
    float wheel_forces_y = 0;
    float wheel_forces_z = 0;
    float wheel_torque_x = 0;
    float wheel_torque_y = 0;
    float wheel_torque_z = 0;
    std::vector<float> _particle_forces;
  };
  struct Outputs {
  public:
    WheelOutput wheel0;
    WheelOutput wheel1;
    WheelOutput wheel2;
    WheelOutput wheel3;
  };
 
  // Interface with the torch implementation
  class NeuralModel {
  public:

    NeuralModel();
    void LoadModel(char* filename, int device);

    void SetInputs(Inputs input);
    void Forward();
    void ForwardDynamic();
    void ForwardCUDATensors();
    Outputs& GetOutputs();

    ~NeuralModel();

  private:
    std::unique_ptr<NeuralModelImpl> Model;
    Inputs _input;
    Outputs _output;
  };

}
}
