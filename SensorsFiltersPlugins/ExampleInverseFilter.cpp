// Copyright (c) 2019 Aptiv
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include <iostream>
#include "Filter.h"
#include <GL/glew.h>
#include <vector>

#include <cstring>

// compile with
// g++ -shared ExampleInverseFilter.cpp -o inverseFilter.dll -fPIC -lGL -lGLEW
class InverseFilter : public Filter {
public:
  InverseFilter() {};

  ~InverseFilter() { tryDeletePixels(); };

  uint8_t *filter(unsigned int textureId, unsigned int width, unsigned int height) {
    // optional, cleaning old CPU-accessible pixels allocation in case the same Filter object is reused
    tryDeletePixels();

    // CUDA computations on texture could be put here
    // -----------------------------------------------
    //
    // -----------------------------------------------

    // get CPU-accessible pixels
    pixels = new uint8_t[width * height * 4 * 4];
    glBindTexture(GL_TEXTURE_2D, textureId);
    glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels);

    // operations on CPU-accessible pixels
    for (int i = 0; i < width * height * 4 * 4; i++) {
      pixels[i] = pixels[i] ^ 0xFFFFFFFFu;
    }

    // return pixels so they can be used in carla
    return pixels;
  }


private:
  void tryDeletePixels() {
    if (pixels) {
      delete pixels;
    }
  }

  // CPU-accessible pixels allocation, stored so we are able to delete it later
  uint8_t *pixels;
};

extern "C" {
Filter *create_object() {
  return new InverseFilter;
}
void destroy_object(Filter *object) {
  delete object;
}
}
