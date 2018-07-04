// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "CoreMinimal.h"

#include <memory>

class AServer;

class FRPCServer
{
public:

  FRPCServer();

  ~FRPCServer();

  void Initialize(AServer &Server, uint16_t Port = 8080u);

  void Run();

  void RunSome();

  void Stop();

private:

  class Pimpl;
  std::unique_ptr<Pimpl> _Pimpl;
};
