#!/usr/bin/env python
""" TL info printer
"""
# Copyright (c) 2021 Computer Vision Center (CVC) at the Universitat Autonoma de
# Barcelona (UAB).
#
# This work is licensed under the terms of the MIT license.
# For a copy, see <https://opensource.org/licenses/MIT>.

# ==============================================================================
# -- imports -------------------------------------------------------------------
# ==============================================================================

import argparse
import imageio

import carla

def get_8bit_texture(image):
    if image is None:
        return carla.TextureFloatColor(0,0)
    height = len(image)
    width = len(image[0])
    texture = carla.TextureColor(width,height)
    for x in range(0,width):
        for y in range(0,height):
            color = image[y][x]
            r = int(color[0])
            g = int(color[1])
            b = int(color[2])
            a = int(color[3])
            texture.set(x, height - y - 1, carla.Color(r,g,b,a))
    return texture

def get_float_texture(image):
    if image is None:
        return carla.TextureFloatColor(0,0)
    height = len(image)
    width = len(image[0])
    texturefloat = carla.TextureFloatColor(width,height)
    for x in range(0,width):
        for y in range(0,height):
            color = image[y][x]
            r = int(color[0])/255.0 * 5
            g = int(color[1])/255.0 * 5
            b = int(color[2])/255.0 * 5
            a = 1.0
            texturefloat.set(x, height - y - 1, carla.FloatColor(r,g,b,a))
    return texturefloat

def main():
    argparser = argparse.ArgumentParser()
    argparser.add_argument(
        '--host',
        metavar='H',
        default='127.0.0.1',
        help='IP of the host server (default: 127.0.0.1)')
    argparser.add_argument(
        '-p', '--port',
        metavar='P',
        default=2000,
        type=int,
        help='TCP port to listen to (default: 2000)')
    argparser.add_argument(
        '-d', '--diffuse',
        type=str,
        default='',
        help='Path to diffuse image to update')
    argparser.add_argument(
        '-o', '--object-name',
        type=str,
        help='Object name')
    argparser.add_argument(
        '-l', '--list',
        action='store_true',
        help='Prints names of all objects in the scene')
    argparser.add_argument(
        '-n', '--normal',
        type=str,
        default='',
        help='Path to normal map to update')
    argparser.add_argument(
        '--ao_roughness_metallic_emissive',
        type=str,
        default='',
        help='Path to normal map to update')
    args = argparser.parse_args()

    client = carla.Client(args.host, args.port)
    client.set_timeout(20.0)

    world = client.get_world()

    if args.list:
        names = world.get_names_of_all_objects()
        for name in names:
            print(name)
        return

    if args.object_name == '':
        print('Error: missing object name to apply texture')
        return

    diffuse = None
    normal = None
    ao_r_m_e = None
    if args.diffuse != '':
        diffuse = imageio.imread(args.diffuse)
    if args.normal != '':
        normal = imageio.imread(args.normal)
    if args.ao_roughness_metallic_emissive != '':
        ao_r_m_e = imageio.imread(args.ao_roughness_metallic_emissive)

    tex_diffuse = get_8bit_texture(diffuse)
    tex_normal = get_float_texture(normal)
    tex_ao_r_m_e = get_float_texture(ao_r_m_e)

    world.apply_textures_to_object(args.object_name, tex_diffuse, carla.TextureFloatColor(0,0), tex_normal, tex_ao_r_m_e)

if __name__ == '__main__':
    main()
