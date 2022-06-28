// Copyright (c) 2022 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB).
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once
#define _GLIBCXX_USE_CXX11_ABI 0

#include <memory>

namespace carla {
namespace learning {

  void test_learning();

  struct NeuralModelImpl;

  class Inputs {
  public:
    float in;
  };
  class Outputs {
  public:
    float out;
  };

  // Interface with the torch implementation
  class NeuralModel {
  public:

    NeuralModel();
    void LoadModel(char* filename);

    void SetIputs(Inputs input);
    void Forward();
    Outputs GetOutputs();

    ~NeuralModel();

  private:
    std::unique_ptr<NeuralModelImpl> Model;
    Inputs _input;
    Outputs _output;
  };

}
}
