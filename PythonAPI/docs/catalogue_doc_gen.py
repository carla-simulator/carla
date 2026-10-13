#!/usr/bin/env python
# -*- coding: utf-8 -*-

# Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma de
# Barcelona (UAB).
#
# This work is licensed under the terms of the MIT license.
# For a copy, see <https://opensource.org/licenses/MIT>.

"""Generates the catalogue pages in Docs/ from a running simulator.

Starts a server itself when none answers, and photographs any catalogue actor
whose image is missing, so a new actor only has to be added to its catalogue.
Writing webp images needs Pillow.
"""

import argparse
import math
import os
import queue
import re
import subprocess
import sys
import time
import unicodedata

import carla

ID_COLOR = '#00a6ed'
TRUE_COLOR = '#99c635'
FALSE_COLOR = '#f16c6c'

IMAGE_DIR = '../img/catalogue'
WEBP_QUALITY = 90

# Town10's own weather, read off a freshly loaded map; the captures start from
# this and change only what they need.
TOWN10_WEATHER = {
    'cloudiness': 30.0,
    'precipitation': 0.0,
    'precipitation_deposits': 0.0,
    'wind_intensity': 0.35,
    'sun_azimuth_angle': 170.0,
    'sun_altitude_angle': 10.0,
    'fog_density': 0.0056,
    'fog_distance': 0.0,
    'fog_falloff': 0.2,
    'wetness': 0.0,
    'scattering_intensity': 0.0,
    'mie_scattering_scale': 0.0,
    'rayleigh_scattering_scale': 0.0331,
    'dust_storm': 0.0,
}

RUN_SERVER = os.path.join('Skills', 'ue58', 'run-carla-ue58-server', 'scripts', 'run_server.sh')


def colored(col, text):
    return '<span style="color:%s;">%s</span>' % (col, text)


def boolean(value):
    return colored(TRUE_COLOR if value else FALSE_COLOR, str(bool(value)))


def attribute(blueprint, name, default=''):
    # as_str only reads String attributes, so the cast follows the declared type.
    if not blueprint.has_attribute(name):
        return default
    attr = blueprint.get_attribute(name)
    casts = {carla.ActorAttributeType.Bool: attr.as_bool,
             carla.ActorAttributeType.Int: attr.as_int,
             carla.ActorAttributeType.Float: attr.as_float}
    return str(casts.get(attr.type, attr.as_str)())


def flag(blueprint, name):
    return blueprint.has_attribute(name) and blueprint.get_attribute(name).as_bool()


def orbit(base, distance, bearing, height, pitch):
    """A camera looking back at @base from @bearing degrees off its heading."""
    yaw = base.rotation.yaw + bearing
    radians = math.radians(yaw)
    return carla.Transform(
        carla.Location(base.location.x + distance * math.cos(radians),
                       base.location.y + distance * math.sin(radians), height),
        carla.Rotation(pitch=pitch, yaw=yaw + 180))


def anchor(heading):
    # The same slug Python-Markdown's toc builds: punctuation goes, then every
    # run of spaces and hyphens collapses into one hyphen.
    text = unicodedata.normalize('NFKD', heading).encode('ascii', 'ignore').decode('ascii')
    text = re.sub(r'[^\w\s-]', '', text).strip().lower()
    return '#' + re.sub(r'[-\s]+', '-', text)


class Entry(object):
    """One catalogue actor: its blueprint, heading, image and detail bullets."""

    def __init__(self, blueprint, heading, image, bullets, view=None):
        self.blueprint = blueprint
        self.heading = heading
        self.image = image
        self.bullets = bullets
        self.view = view
        self.subgroup = None
        self.capture = None


class Overview(object):
    """The looping animation a catalogue opens with, one actor fading into the next."""

    def __init__(self, image, hold_ms=1200, fade_steps=4, fade_ms=60, scale=1.0):
        self.image = image
        self.hold_ms = hold_ms
        self.fade_steps = fade_steps
        self.fade_ms = fade_ms
        self.scale = scale


class Capture(object):
    """Where and how a catalogue photographs its actors."""

    def __init__(self, town, actor, camera, resolution, weather, settle_ticks=60,
                 camera_attributes=None, on_blueprint=None, on_spawn=None):
        self.town = town
        self.actor = actor
        self.camera = camera
        self.resolution = resolution
        self.weather = weather
        self.settle_ticks = settle_ticks
        self.camera_attributes = camera_attributes or {}
        self.on_blueprint = on_blueprint
        self.on_spawn = on_spawn

    def camera_for(self, entry):
        """One camera for the whole catalogue, or one per entry.view."""
        if isinstance(self.camera, dict):
            return self.camera[entry.view]
        return self.camera

    def settings(self):
        """The same block the vehicle catalogue keeps by hand, as a comment."""
        if self.actor is None:
            return ['The images in this page were taken by hand, one scene each;',
                    '%s only writes the page around them.' % SCRIPT_NAME]
        lines = ['The images in this page were captured by %s with these settings:' % SCRIPT_NAME,
                 '',
                 'Town: %s' % self.town,
                 '',
                 'Actor transform: %s' % self.actor]
        if isinstance(self.camera, dict):
            lines += ['Camera transform, %s: %s' % item for item in sorted(self.camera.items())]
        else:
            lines.append('Camera transform: %s' % self.camera)
        lines += ['Camera resolution: %dx%d' % self.resolution]
        lines += ['camera.%s = %s' % item for item in sorted(self.camera_attributes.items())]
        lines.append('')
        lines += ['weather.%s = %s' % item for item in sorted(self.weather.items())]
        return lines


# ---------------------------------------------------------------------------
# pedestrians


def pedestrian_entries(library):
    """Splits the walkers into adults, children and special, in id order.

    A walker with a special_type is special whatever its age, and takes its
    heading from that type, so 'police' reads 'Police pedestrian 1'.
    """
    order = ['Adults', 'Children', 'Special']
    groups = dict((name, []) for name in order)
    counters = {}
    for blueprint in sorted(library.filter('walker.pedestrian.*'), key=lambda b: b.id):
        special = attribute(blueprint, 'special_type')
        age = attribute(blueprint, 'age', 'adult')
        if special:
            group, kind = 'Special', special.replace('_', ' ').capitalize()
        elif age == 'child':
            group, kind = 'Children', 'Child'
        else:
            group, kind = 'Adults', 'Adult'

        counters[kind] = counters.get(kind, 0) + 1
        heading = '%s pedestrian %d' % (kind, counters[kind])
        name = blueprint.id.split('.')[-1]

        bullets = [
            '__Blueprint ID__: ' + colored(ID_COLOR, blueprint.id),
            '__Age__: ' + age,
            '__Gender__: ' + attribute(blueprint, 'gender', 'unspecified'),
            '__Generation__: ' + attribute(blueprint, 'generation', '-1'),
        ]
        if special:
            bullets.append('__Special type__: ' + special)
        bullets.append('__Can use wheelchair__: ' + boolean(flag(blueprint, 'can_use_wheelchair')))

        groups[group].append(Entry(
            blueprint, heading,
            '%s/pedestrians/pedestrian_%s.webp' % (IMAGE_DIR, name), bullets))

    return [(name, groups[name]) for name in order if groups[name]]


PEDESTRIAN_OVERVIEW = Overview(
    image='%s/pedestrians/pedestrians_overview.webp' % IMAGE_DIR)


PEDESTRIAN_CAPTURE = Capture(
    town='Town10HD_Opt',
    actor=carla.Transform(carla.Location(x=-88.1, y=-73.6, z=1.0), carla.Rotation(yaw=90)),
    camera=carla.Transform(carla.Location(x=-88.1, y=-70.8, z=1.55), carla.Rotation(pitch=-5, yaw=270)),
    resolution=(1848, 1016),
    weather=dict(TOWN10_WEATHER, sun_altitude_angle=50, sun_azimuth_angle=260,
                 cloudiness=20, wetness=10))


# ---------------------------------------------------------------------------
# vehicles


# Make and Model in the catalogue are not always in that order, and a few read
# as the model only, so the headings the page had are kept by blueprint id.
VEHICLE_NAMES = {
    'vehicle.ambulance.ford': 'Ford - Ambulance',
    'vehicle.bh.crossbike': 'BH - Crossbike',
    'vehicle.carlacola.actors': 'CARLA Motors - CarlaCola',
    'vehicle.diamondback.century': 'Diamondback - Century',
    'vehicle.dodge.charger': 'Dodge - Charger',
    'vehicle.dodgecop.charger': 'Dodge - Police Charger',
    'vehicle.firetruck.actors': 'CARLA Motors - Firetruck',
    'vehicle.fuso.mitsubishi': 'Mitsubishi - Fusorosa',
    'vehicle.gazelle.omafiets': 'Gazelle - Omafiets',
    'vehicle.harley-davidson.low_rider': 'Harley-Davidson - Low Rider',
    'vehicle.kawasaki.ninja': 'Kawasaki - Ninja',
    'vehicle.kia.niro': 'Kia - Niro',
    'vehicle.lincoln.mkz': 'Lincoln - MKZ',
    'vehicle.mini.cooper': 'Mini - Cooper S',
    'vehicle.miningtruck.miningtruck': 'Mining earth mover',
    'vehicle.nissan.patrol': 'Nissan - Patrol',
    'vehicle.sprinter.mercedes': 'Mercedes - Sprinter',
    'vehicle.taxi.ford': 'Ford - Crown (taxi)',
    'vehicle.tesla.model3': 'Tesla - Model 3',
    'vehicle.ue4.audi.tt': 'Audi - TT',
    'vehicle.ue4.chevrolet.impala': 'Chevrolet - Impala',
    'vehicle.ue4.ford.crown': 'Ford - Crown',
    'vehicle.ue4.ford.mustang': 'Ford - Mustang',
    'vehicle.ue4.mercedes.ccc': 'Mercedes - Coupe',
    'vehicle.vespa.zx125': 'Vespa - ZX125',
    'vehicle.yamaha.yzf': 'Yamaha - YZF',
}

# base_type -> the section it goes in, in the order the page lists them, and
# which of the three framings fits a body that size.
VEHICLE_GROUPS = [
    ('car', 'Cars', 'standard'),
    ('motorcycle', 'Motorcycles', 'small'),
    ('bicycle', 'Bicycles', 'small'),
    ('truck', 'Trucks', 'large'),
    ('van', 'Vans', 'standard'),
    ('bus', 'Buses', 'large'),
    ('industrial', 'Industrial', 'large'),
]


def vehicle_entries(library):
    """Groups the vehicles by base_type, in the order the page lists them."""
    sections = [(base, title, view) for base, title, view in VEHICLE_GROUPS]
    groups = dict((title, []) for _, title, _ in sections)
    views = dict((base, view) for base, _, view in sections)
    titles = dict((base, title) for base, title, _ in sections)

    for blueprint in sorted(library.filter('vehicle.*'), key=lambda b: b.id):
        base = attribute(blueprint, 'base_type', 'car')
        title = titles.get(base)
        if title is None:
            print('  %s: unknown base_type %r, listed under Cars' % (blueprint.id, base))
            base, title = 'car', 'Cars'
        name = blueprint.id.split('.', 1)[1].replace('.', '_')
        special = attribute(blueprint, 'special_type')

        bullets = [
            '__Blueprint ID__: ' + colored(ID_COLOR, blueprint.id),
            '__Base type__: ' + base,
        ]
        if special:
            bullets.append('__Special type__: ' + special)
        bullets += [
            '__Generation__: ' + attribute(blueprint, 'generation', '-1'),
            '__Number of wheels__: ' + attribute(blueprint, 'number_of_wheels', '-'),
            '__Has lights__: ' + boolean(flag(blueprint, 'has_lights')),
            '__Has opening doors__: ' + boolean(flag(blueprint, 'has_dynamic_doors')),
        ]

        groups[title].append(Entry(
            blueprint, VEHICLE_NAMES.get(blueprint.id, pretty_name(blueprint.id)),
            '%s/vehicles/%s.webp' % (IMAGE_DIR, name), bullets, view=views[base]))

    return [(title, groups[title]) for _, title, _ in sections if groups[title]]


def pretty_name(blueprint_id):
    parts = blueprint_id.split('.')[1:]
    return ' - '.join(part.replace('_', ' ').title() for part in parts[-2:])


def luminance(value):
    red, green, blue = (int(part) for part in value.split(','))
    return 0.2126 * red + 0.7152 * green + 0.0722 * blue


def pick_color(blueprint):
    """Keeps the default colour unless it is too dark to read the bodywork."""
    if not blueprint.has_attribute('color'):
        return
    recommended = list(blueprint.get_attribute('color').recommended_values)
    if not recommended:
        return
    chosen = recommended[0]
    if luminance(chosen) < 60:
        chosen = max(recommended, key=luminance)
    blueprint.set_attribute('color', chosen)


def steer_wheels(actor):
    """The catalogue shots show the wheels turned, as the hand-made ones did."""
    control = actor.get_control()
    control.steer = -0.25
    actor.apply_control(control)


VEHICLE_OVERVIEW = Overview(image='%s/vehicles/vehicle_montage.webp' % IMAGE_DIR)

# The spot and the three framings the hand-made page recorded, kept as they
# were; only the spawn height is raised so the vehicle drops onto the road.
VEHICLE_SPOT = carla.Transform(carla.Location(x=-46.885479, y=20.083447, z=0.3),
                               carla.Rotation(pitch=-0.000034, yaw=141.974243))

VEHICLE_CAPTURE = Capture(
    town='Town10HD_Opt',
    actor=VEHICLE_SPOT,
    camera={
        'small': carla.Transform(
            carla.Location(x=-47.696186, y=24.049326, z=1.471929),
            carla.Rotation(pitch=-10.843717, yaw=-77.215683, roll=0.000139)),
        'standard': carla.Transform(
            carla.Location(x=-48.672256, y=24.830288, z=1.722733),
            carla.Rotation(pitch=-13.396630, yaw=-75.692039, roll=0.000119)),
        'large': carla.Transform(
            carla.Location(x=-49.470921, y=27.835310, z=2.931721),
            carla.Rotation(pitch=-13.396630, yaw=-75.691978, roll=0.000119)),
    },
    resolution=(1920, 1080),
    # Town10's own weather, with the sun raised and turned so the junction is
    # lit rather than in shadow, and the sky cleared. Every field is listed: the
    # weather is built from scratch so a run never inherits the server's.
    weather=dict(TOWN10_WEATHER, sun_altitude_angle=50, sun_azimuth_angle=350,
                 cloudiness=0, scattering_intensity=0, mie_scattering_scale=0.0),
    # The default profile auto-exposes until white paint clips and blooms into
    # a halo; GoPro holds the highlights, and no weather setting can.
    camera_attributes={'post_process_profile': 'GoPro'},
    on_blueprint=pick_color,
    on_spawn=steer_wheels)


# The mine truck keeps its hand-made render, which is far better than anything
# a drive-by capture gets. TODO: if it ever has to be reshot, it is only
# registered on its own map, and at 12 x 7 x 6 m it needs a camera much further
# back than anything in town -- the spot below worked, with the camera at
# Location(x=-66.924, y=-221.587, z=76.322), Rotation(pitch=-13.4, yaw=213.302).
#   carla.Transform(carla.Location(x=-85.311592, y=-233.666946, z=75.030807),
#                   carla.Rotation(yaw=66.802))
MINE_CAPTURE = Capture(
    town='Mine_01', actor=None, camera=None, resolution=(1920, 1080), weather={})


# ---------------------------------------------------------------------------
# props


# The headings the hand-written page used, kept so the generated one reads the
# same; a prop that is not listed falls back to its blueprint id.
PROP_NAMES = {
    'advertisement': 'Advertisement',
    'atm': 'ATM',
    'barbeque': 'Barbeque',
    'barrel': 'Barrel',
    'bench01': 'Bench 01',
    'bench02': 'Bench 02',
    'bikehelmet': 'Bike helmet',
    'briefcase': 'Briefcase',
    'brokentile01': 'Brokentile 01',
    'brokentile02': 'Brokentile 02',
    'brokentile03': 'Brokentile 03',
    'brokentile04': 'Brokentile 04',
    'busstop': 'Bus stop',
    'calibrator': 'Calibrator',
    'chainbarrier': 'Chain barrier',
    'chainbarrierend': 'Chain barrier end',
    'colacan': 'Cola can',
    'constructioncone': 'Construction cone',
    'container': 'Garbage container',
    'creasedbox01': 'Creased box 01',
    'dirtdebris01': 'Dirt debris 01',
    'dirtdebris02': 'Dirt debris 02',
    'dirtdebris03': 'Dirt debris 03',
    'doghouse': 'Doghouse',
    'fountain': 'Fountain',
    'garbage01': 'Garbage 01',
    'garbage02': 'Garbage 02',
    'garbage03': 'Garbage 03',
    'garbage04': 'Garbage 04',
    'garbage05': 'Garbage 05',
    'garbage06': 'Garbage 06',
    'gnome': 'Gnome',
    'guitarcase': 'Guitarcase',
    'haybale': 'Haybale',
    'kiosk_01': 'Kiosk 01',
    'mailbox': 'Mailbox',
    'maptable': 'Map table',
    'mobile': 'Mobile',
    'motorhelmet': 'Motor helmet',
    'pergola': 'Pergola',
    'plantpot01': 'Plantpot 01',
    'plantpot02': 'Plantpot 02',
    'plantpot03': 'Plantpot 03',
    'plantpot04': 'Plantpot 04',
    'plantpot05': 'Plantpot 05',
    'plantpot06': 'Plantpot 06',
    'plantpot07': 'Plantpot 07',
    'plasticbag': 'Plastic bag',
    'plasticchair': 'Plastic chair',
    'plastictable': 'Plastic table',
    'platformgarbage01': 'Garbage platform 01',
    'purse': 'Purse',
    'shoppingbag': 'Shopping bag',
    'shoppingcart': 'Shopping cart',
    'shoppingtrolley': 'Shopping trolley',
    'streetbarrier': 'Street barrier',
    'streetfountain': 'Street fountain',
    'streetsign': 'Street sign',
    'streetsign01': 'Street sign 01',
    'streetsign04': 'Street sign 04',
    'swing': 'Swing',
    'swingcouch': 'Swingcouch',
    'trafficcone01': 'Traffic cone 01',
    'trafficcone02': 'Traffic cone 02',
    'trafficwarning': 'Traffic warning',
    'trampoline': 'Trampoline',
    'trashbag': 'Trash bag',
    'trashcan01': 'Trash can 01',
    'trashcan02': 'Trash can 02',
    'trashcan03': 'Trash can 03',
    'trashcan04': 'Trash can 04',
    'travelcase': 'Travel case',
    'vendingmachine': 'Vending machine',
    'warningaccident': 'Warning accident',
    'warningconstruction': 'Warning construction',
}


# base_type -> the section it goes in, in the order the page lists them.
PROP_GROUPS = [
    ('containers', 'Containers'),
    ('garbage', 'Garbage'),
    ('park_garden', 'Park / garden'),
    ('personal', 'Personal'),
    ('road', 'Road'),
    ('sidewalk', 'Sidewalk'),
    ('utility', 'Utility'),
]


def prop_entries(library):
    """Groups the props by base_type, and inside each one by subtype."""
    titles = dict(PROP_GROUPS)
    groups = dict((title, []) for _, title in PROP_GROUPS)

    for blueprint in sorted(library.filter('static.prop.*'), key=lambda b: b.id):
        base = attribute(blueprint, 'base_type')
        title = titles.get(base)
        if title is None:
            continue
        name = blueprint.id.split('.', 2)[2]
        subtype = attribute(blueprint, 'subtype')
        bullets = [
            '__Blueprint ID__: ' + colored(ID_COLOR, blueprint.id),
            '__Base type__: ' + base,
            '__Subtype__: ' + subtype,
            '__Size__: ' + attribute(blueprint, 'size', 'unknown'),
        ]
        entry = Entry(blueprint, PROP_NAMES.get(name, name.replace('_', ' ').capitalize()),
                      '%s/props/static_prop_%s.webp' % (IMAGE_DIR, name), bullets)
        entry.subgroup = subtype.replace('_', ' ').capitalize()
        groups[title].append(entry)

    for entries in groups.values():
        entries.sort(key=lambda e: (e.subgroup, e.blueprint.id))
    return [(title, groups[title]) for _, title in PROP_GROUPS if groups[title]]


# The props were photographed by hand, one scene each; nothing here reshoots them.
PROP_CAPTURE = Capture(
    town='Town10HD_Opt', actor=None, camera=None, resolution=(1920, 1080), weather={})


CATALOGUES = {
    'pedestrians': ('Pedestrian catalogue', pedestrian_entries,
                    [PEDESTRIAN_CAPTURE], PEDESTRIAN_OVERVIEW),
    'vehicles': ('Vehicle catalogue', vehicle_entries,
                 [VEHICLE_CAPTURE, MINE_CAPTURE], VEHICLE_OVERVIEW),
    'props': ('Props catalogue', prop_entries, [PROP_CAPTURE], None),
}

SCRIPT_NAME = 'PythonAPI/docs/catalogue_doc_gen.py'


# ---------------------------------------------------------------------------
# rendering


def render(title, groups, capture):
    out = ['<!---', '']
    out += capture.settings()
    out += ['', 'Do not edit this page by hand, %s overwrites it.' % SCRIPT_NAME,
            '', '--->', '', '# ' + title, '']

    for name, entries in groups:
        out.append('* __%s__' % name)
        subgroup = None
        for entry in entries:
            if entry.subgroup and entry.subgroup != subgroup:
                subgroup = entry.subgroup
                out.append('\t* __%s__' % subgroup)
            indent = '\t\t' if entry.subgroup else '\t'
            out.append('%s* [__%s__](%s)' % (indent, entry.heading, anchor(entry.heading)))
    out.append('')

    for name, entries in groups:
        out += ['---', '', '## ' + name, '']
        for entry in entries:
            out += ['### ' + entry.heading, '']
            if entry.image:
                out += ['![%s](%s)' % (os.path.basename(entry.image).rsplit('.', 1)[0],
                                       entry.image), '']
            out += ['* ' + bullet for bullet in entry.bullets]
            out.append('')

    return '\n'.join(out)


# ---------------------------------------------------------------------------
# the simulator


def connect(host, port, attempts=1, timeout=10.0):
    # A server that is still loading its map holds the port open but refuses
    # the connection, so a single probe cannot tell it from no server at all.
    for attempt in range(attempts):
        client = carla.Client(host, port)
        client.set_timeout(timeout)
        try:
            client.get_server_version()
            return client
        except RuntimeError:
            if attempt + 1 < attempts:
                time.sleep(5.0)
    return None


def ensure_server(host, port, root):
    """Connects, starting a server in -game mode when nothing answers."""
    client = connect(host, port, attempts=4)
    if client is not None:
        return client, False

    script = os.path.join(root, RUN_SERVER)
    if host not in ('127.0.0.1', 'localhost') or not os.path.isfile(script):
        print('[ERROR] No simulator on %s:%d and none can be started here' % (host, port))
        sys.exit(1)

    print('No simulator on %s:%d, starting one...' % (host, port))
    env = dict(os.environ, DETACH='1', PORT=str(port))
    if subprocess.call(['bash', script, 'game'], cwd=root, env=env) != 0:
        print('[ERROR] Could not start a simulator')
        sys.exit(1)

    for _ in range(60):
        client = connect(host, port)
        if client is not None:
            return client, True
        time.sleep(5.0)
    print('[ERROR] Started a simulator but it never answered on port %d' % port)
    sys.exit(1)


def stop_server(root):
    subprocess.call(['bash', os.path.join(root, RUN_SERVER), 'stop'], cwd=root)


def save_image(image, path):
    """Saves a camera image, through Pillow for the formats carla cannot write."""
    directory = os.path.dirname(path)
    if directory and not os.path.isdir(directory):
        os.makedirs(directory)
    if not path.endswith('.webp'):
        image.save_to_disk(path)
        return
    from PIL import Image
    frame = Image.frombuffer('RGBA', (image.width, image.height),
                             image.raw_data, 'raw', 'BGRA', 0, 1)
    frame.convert('RGB').save(path, 'WEBP', quality=WEBP_QUALITY, method=6)


def prepare(client, capture):
    """Puts the world in the state every shot is taken in."""
    world = client.load_world_if_different(capture.town)

    settings = world.get_settings()
    original = (world.get_settings(), world.get_weather())
    settings.synchronous_mode = True
    settings.fixed_delta_seconds = 0.05
    world.apply_settings(settings)

    world.set_weather(carla.WeatherParameters(**capture.weather))

    camera_bp = world.get_blueprint_library().find('sensor.camera.rgb')
    camera_bp.set_attribute('image_size_x', str(capture.resolution[0]))
    camera_bp.set_attribute('image_size_y', str(capture.resolution[1]))
    for name, value in capture.camera_attributes.items():
        if not camera_bp.has_attribute(name):
            print('  the camera has no %r attribute, ignored' % name)
            continue
        camera_bp.set_attribute(name, str(value))
    return world, camera_bp, original


def restore(world, original):
    # Always hand the world back asynchronous: a synchronous world with nobody
    # ticking it freezes every other client that connects to the server.
    settings, weather = original
    settings.synchronous_mode = False
    settings.fixed_delta_seconds = None
    world.set_weather(weather)
    world.apply_settings(settings)


def shoot(world, camera_bp, transform, ticks):
    """Returns the last frame of the settling window.

    The textures stream in and the exposure adapts over the first frames, so
    the earlier ones are thrown away.
    """
    camera = world.spawn_actor(camera_bp, transform)
    images = queue.Queue()
    camera.listen(images.put)
    for _ in range(ticks):
        world.tick()
    image = None
    while not images.empty():
        image = images.get()
    camera.destroy()
    return image


def photograph(client, capture, entries, out_dir):
    """Spawns each actor at the capture transform and saves one settled frame."""
    world, camera_bp, original = prepare(client, capture)
    library = world.get_blueprint_library()
    try:
        for entry in entries:
            blueprint = library.find(entry.blueprint.id)
            if capture.on_blueprint is not None:
                capture.on_blueprint(blueprint)
            actor = world.try_spawn_actor(blueprint, capture.actor)
            if actor is None:
                print('  %s: could not spawn' % entry.blueprint.id)
                continue
            if capture.on_spawn is not None:
                capture.on_spawn(actor)
            image = shoot(world, camera_bp, capture.camera_for(entry), capture.settle_ticks)
            if image is not None:
                save_image(image, os.path.join(out_dir, entry.image.replace('../', '')))
                print('  %s -> %s' % (entry.blueprint.id, os.path.basename(entry.image)))
            actor.destroy()
            world.tick()
    finally:
        restore(world, original)


def build_overview(overview, entries, out_dir):
    """Joins the actor images into one looping animation, fading between them."""
    from PIL import Image

    def load(entry):
        frame = Image.open(os.path.join(out_dir, entry.image.replace('../', ''))).convert('RGB')
        if overview.scale != 1.0:
            size = (int(frame.width * overview.scale), int(frame.height * overview.scale))
            frame = frame.resize(size, Image.LANCZOS)
        return frame

    frames, durations = [], []
    shots = [load(entry) for entry in entries if entry.image]
    for index, shot in enumerate(shots):
        frames.append(shot)
        durations.append(overview.hold_ms)
        nxt = shots[(index + 1) % len(shots)]
        for step in range(1, overview.fade_steps + 1):
            frames.append(Image.blend(shot, nxt, step / float(overview.fade_steps + 1)))
            durations.append(overview.fade_ms)

    path = os.path.join(out_dir, overview.image.replace('../', ''))
    frames[0].save(path, save_all=True, append_images=frames[1:],
                   duration=durations, loop=0, quality=80, method=4)
    print('  %d actors, %d frames -> %s (%.1f MB)'
          % (len(shots), len(frames), os.path.basename(overview.image),
             os.path.getsize(path) / 1048576.0))


def main():
    docs_dir = os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'Docs')
    root = os.path.normpath(os.path.join(docs_dir, '..'))

    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--host', default='127.0.0.1')
    parser.add_argument('--port', default=2000, type=int)
    parser.add_argument('--catalogue', default=sorted(CATALOGUES), nargs='+',
                        choices=sorted(CATALOGUES))
    parser.add_argument('--out-dir', default=os.path.normpath(docs_dir))
    parser.add_argument('--recapture', action='store_true',
                        help='photograph every actor, not only the ones without an image')
    args = parser.parse_args()

    client, started = ensure_server(args.host, args.port, root)
    client.set_timeout(60.0)
    library = client.get_world().get_blueprint_library()

    try:
        for name in args.catalogue:
            title, collect, captures, overview = CATALOGUES[name]

            # A catalogue can span several maps: an actor only registered on
            # its own map is collected, and later shot, on that map.
            groups, seen = [], set()
            for capture in captures:
                library = client.load_world_if_different(
                    capture.town).get_blueprint_library()
                for group_title, found in collect(library):
                    fresh = []
                    for entry in found:
                        if entry.blueprint.id in seen:
                            continue
                        seen.add(entry.blueprint.id)
                        entry.capture = capture
                        fresh.append(entry)
                    if not fresh:
                        continue
                    existing = [g for g in groups if g[0] == group_title]
                    if existing:
                        existing[0][1].extend(fresh)
                    else:
                        groups.append((group_title, fresh))

            entries = [entry for _, found in groups for entry in found]

            def absent(image):
                return args.recapture or not os.path.isfile(
                    os.path.join(args.out_dir, image.replace('../', '')))

            for capture in captures:
                # --recapture cannot reshoot a hand-made image, so for those
                # only a file that is really gone counts as missing.
                gone = absent if capture.actor is not None else (
                    lambda image: not os.path.isfile(
                        os.path.join(args.out_dir, image.replace('../', ''))))
                missing = [entry for entry in entries
                           if entry.capture is capture and gone(entry.image)]
                if not missing:
                    continue
                if capture.actor is None:
                    # A catalogue photographed by hand: list what is missing and
                    # leave the entry without a picture rather than a dead link.
                    print('  %d actor(s) without an image: %s'
                          % (len(missing), ', '.join(e.blueprint.id for e in missing)))
                    for entry in missing:
                        entry.image = None
                    continue
                print('  photographing %d actor(s) on %s' % (len(missing), capture.town))
                photograph(client, capture, missing, args.out_dir)

            path = os.path.join(args.out_dir, 'catalogue_%s.md' % name)
            with open(path, 'w') as md_file:
                md_file.write(render(title, groups, captures[0]) + '\n')
            print('%s: %d entries in %d groups -> %s'
                  % (name, len(entries), len(groups), os.path.relpath(path)))

            if overview is not None:
                print('  building the overview')
                build_overview(overview, entries, args.out_dir)
    finally:
        if started:
            stop_server(root)


if __name__ == '__main__':
    main()
