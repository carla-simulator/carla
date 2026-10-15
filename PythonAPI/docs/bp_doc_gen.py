#!/usr/bin/env python
# -*- coding: utf-8 -*-

# Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma de
# Barcelona (UAB).
#
# This work is licensed under the terms of the MIT license.
# For a copy, see <https://opensource.org/licenses/MIT>.

"""Generates Docs/bp_library.md from the blueprints of a running server."""

import argparse
import datetime
import os
import sys

import carla

# Blueprints that exist only to exercise the spawning pipeline.
DENYLIST = {'blueprint.trafficlightexample'}

SECTIONS = [
    ('vehicle', 'Vehicles', 'Spawnable vehicles. See the [vehicle catalogue](catalogue_vehicles.md) for images and [Actors](core_actors.md) for how to drive them.'),
    ('walker', 'Pedestrians', 'Spawnable pedestrians. See the [pedestrian catalogue](catalogue_pedestrians.md); they are driven by a `controller.ai.walker`.'),
    ('controller', 'Controllers', 'Actors that drive another actor. `controller.ai.walker` steers a pedestrian towards a destination.'),
    ('sensor', 'Sensors', 'Sensors attachable to an actor. Each one is documented in [Sensors reference](ref_sensors.md).'),
    ('static', 'Props', 'Static meshes that can be spawned as actors. See the [prop catalogue](catalogue_props.md).'),
]


def code(buf):
    return '`{}`'.format(buf)


def escape(buf):
    """Pipes and newlines break a Markdown table cell."""
    return str(buf).replace('|', '\\|').replace('\n', ' ')


def attribute_rows(blueprint):
    rows = []
    for attr in sorted(blueprint, key=lambda x: x.id):
        default = escape(attr.as_str())
        values = ', '.join(code(escape(v)) for v in attr.recommended_values)
        rows.append('| {} | {} | {} | {} | {} |'.format(
            code(attr.id),
            str(attr.type).replace('ActorAttributeType.', ''),
            code(default) if default else '',
            values,
            'yes' if attr.is_modifiable else 'no'))
    return rows


def generate_bp_docs(host, port, timeout):
    """Returns the markdown of the blueprint library of the server at host:port."""

    print('Generating API blueprint documentation...')
    client = carla.Client(host, port)
    client.set_timeout(timeout)
    world = client.get_world()

    blueprints = [bp for bp in world.get_blueprint_library().filter('*') if bp.id not in DENYLIST]
    by_type = {}
    for bp in sorted(blueprints, key=lambda x: x.id):
        by_type.setdefault(bp.id.split('.')[0], []).append(bp)

    out = ['# Blueprint library', '']
    out.append('The blueprint library ([`carla.BlueprintLibrary`](python_api.md#carla.BlueprintLibrary)) lists every actor '
               '([`carla.ActorBlueprint`](python_api.md#carla.ActorBlueprint)) that can be spawned, with the attributes '
               '([`carla.ActorAttribute`](python_api.md#carla.ActorAttribute)) each one accepts. '
               'Start with the [introduction to blueprints](core_actors.md).')
    out.append('')
    out.append('Generated from CARLA {} on {}. A server built with other options, or with content packs loaded, lists a different set.'.format(
        client.get_server_version(), datetime.date.today().isoformat()))
    out.append('')
    out.append('```py')
    out.append("blueprints = [bp for bp in world.get_blueprint_library().filter('*')]")
    out.append('for blueprint in blueprints:')
    out.append('   print(blueprint.id)')
    out.append('   for attr in blueprint:')
    out.append("       print('  - {}'.format(attr))")
    out.append('```')
    out.append('')

    sections = [s for s in SECTIONS if s[0] in by_type]
    sections += [(k, k.capitalize(), '') for k in sorted(by_type) if k not in dict((s[0], s) for s in SECTIONS)]

    out.append('- ' + '\n- '.join('[{}](#{}) ({} blueprints)'.format(
        title, title.lower().replace(' ', '-'), len(by_type[key])) for key, title, _ in sections))
    out.append('')

    for key, title, intro in sections:
        out.append('---')
        out.append('')
        out.append('## {}'.format(title))
        out.append('')
        if intro:
            out.append(intro)
            out.append('')
        for bp in by_type[key]:
            out.append('### {}'.format(bp.id))
            out.append('')
            rows = attribute_rows(bp)
            if rows:
                out.append('| Attribute | Type | Default | Values | Modifiable |')
                out.append('| --- | --- | --- | --- | --- |')
                out.extend(rows)
            else:
                out.append('No attributes.')
            out.append('')

    return '\n'.join(out) + '\n'


def main():
    argparser = argparse.ArgumentParser(description=__doc__)
    argparser.add_argument('--host', default='127.0.0.1', help='IP of the host server (default: 127.0.0.1)')
    argparser.add_argument('-p', '--port', default=2000, type=int, help='TCP port to listen to (default: 2000)')
    argparser.add_argument('-t', '--timeout', default=10.0, type=float, help='connection timeout in seconds (default: 10.0)')
    args = argparser.parse_args()

    script_path = os.path.dirname(os.path.abspath(__file__))

    try:
        docs = generate_bp_docs(args.host, args.port, args.timeout)
    except RuntimeError as error:
        print("\n  [ERROR] Can't establish connection with the simulator: {}".format(error))
        print('  Start a server on {}:{} and run this script again.\n'.format(args.host, args.port))
        sys.exit(1)

    with open(os.path.join(script_path, '../../Docs/bp_library.md'), 'w') as md_file:
        md_file.write(docs)
    print('Done!')


if __name__ == '__main__':
    main()
