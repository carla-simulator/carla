#!/usr/bin/env python

# Copyright (c) 2024 Computer Vision Center (CVC) at the Universitat Autonoma de
# Barcelona (UAB).
#
# This work is licensed under the terms of the MIT license.
# For a copy, see <https://opensource.org/licenses/MIT>.

# ==============================================================================
# -- imports -------------------------------------------------------------------
# ==============================================================================

import carla
import argparse
import numpy as np

DEFAULT_HOST = '127.0.0.1'
DEFAULT_PORT = 2000

def get_8bit_texture(image, w, h):
    if image is None:
      return carla.TextureFloatColor(0, 0)
    texture = carla.TextureColor(w, h)
    for x in range(0, w):
        for y in range(0, h):
            color = image[y][x]
            r = int(color[0])
            g = int(color[1])
            b = int(color[2])
            a = int(color[3])
            texture.set(x, h - y - 1, carla.Color(r,g,b,a))
    return texture

def get_float_texture(image, w, h):
    if image is None:
      return carla.TextureFloatColor(0, 0)
    texturefloat = carla.TextureFloatColor(w,h)
    for x in range(0, w):
        for y in range(0, h):
            color = image[y][x]
            r = int(color[0])/255.0 * 5
            g = int(color[1])/255.0 * 5
            b = int(color[2])/255.0 * 5
            a = 1.0
            texturefloat.set(x, h - y - 1, carla.FloatColor(r,g,b,a))
    return texturefloat

def make_texture(h, w, cc, type = float):
  return np.reshape(
    [ type(0) for i in range(0, h * w * cc) ],
    [ w, h, cc ])

def main():
    ec = 0
    try:
      argparser = argparse.ArgumentParser()
      argparser.add_argument(
          '--host',
          metavar='H',
          default=DEFAULT_HOST,
          help=f'IP of the host server (default: {DEFAULT_HOST})')
      argparser.add_argument(
          '-p', '--port',
          metavar='P',
          default=DEFAULT_PORT,
          type=int,
          help=f'TCP port to listen to (default: {DEFAULT_PORT})')
      args = argparser.parse_args()

      client = carla.Client(args.host, args.port)
      client.set_timeout(20.0)
      world = client.get_world()
      h = 256
      w = 256
      print('Creating diffuse noise...')
      diffuse = make_texture(w, h, 4)
      print('Creating normal noise...')
      normal = make_texture(w, h, 3)
      print('Creating aorme noise...')
      aorme = make_texture(w, h, 3)
      print('Creating diffuse texture...')
      tex_diffuse = get_8bit_texture(diffuse, w, h)
      print('Creating normal texture...')
      tex_normal = get_float_texture(normal, w, h)
      print('Creating aorme texture...')
      tex_ao_r_m_e = get_float_texture(aorme, w, h)
      this_name = None
      this_name_index = 0
      name_list = world.get_names_of_all_objects()
      for e in name_list:
        this_name = e
        print(f'Applying random test texture to {e}')
        world.apply_textures_to_object(
          e,
          tex_diffuse,
          carla.TextureFloatColor(0,0),
          tex_normal,
          tex_ao_r_m_e)
    except Exception as e:
      ec = -1
      print('Exception caught while running texture change test.')
      print(f' Actor Name: "{this_name}"')
      print(f' Actor Index: {this_name_index}')
    finally:
      if this_name_index != len(name_list):
         print()
    exit(ec)

if __name__ == '__main__':
    main()
