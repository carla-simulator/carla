// TODO header?

#pragma once
#include "Kernel.cuh"

#include <cstdint>

class Filter {
public:
  Filter() {}

  virtual ~Filter() {}

  virtual uint8_t *filter(unsigned int textureId, unsigned int width, unsigned int height) = 0;
};
