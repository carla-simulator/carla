// Copyright (c) 2019 Aptiv
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include <cstdint>

class Filter {
public:
  Filter() {}

  virtual ~Filter() {}

  virtual uint8_t *filter(unsigned int textureId, unsigned int width, unsigned int height) = 0;
};
