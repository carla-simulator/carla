// Copyright (c) 2019 Aptiv
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once
#include "cuda_runtime.h"
#include "device_launch_parameters.h"
#include <iostream>
#include <iostream>
#include <GL/glew.h>
#include <vector>

#include <cstring>
#include <cuda_gl_interop.h>

extern "C" {
  void negatif_parallel(unsigned int textureId, int width, int height);
}
