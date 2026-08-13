#!/usr/bin/env python
"""
Script to add new walkers (pedestrians) to the walker factory

Counterpart of add_vehicle_to_vehicle_factory.py. Run from inside the editor:

    UE4Editor CarlaUE4.uproject -ExecutePythonScript="add_walker_to_walker_factory.py
        -w /Game/Carla/Blueprints/Walkers/BP_Walker_MyKid --gender male --age child
        --speed 0.0,1.1,2.0"

The entry is appended to WalkerFactory's `Pedestrians` array and the blueprint is
compiled and saved, which is what the Compile and Save buttons do in the editor.

NOTE: `Pedestrians` must be a blueprint MEMBER variable of WalkerFactory. Older
content keeps the list in `Walkers`, a variable LOCAL to the GenerateDefinitions
function; function locals are not class properties, so reflection cannot reach them
and this script cannot register anything until the array is promoted to a member
variable (as VehicleFactory already does with `Vehicles`).
"""
import argparse
import json
import unreal

WALKER_FACTORY_PATH = '/Game/Carla/Blueprints/Walkers/WalkerFactory'

GENDERS = {
    'male': unreal.PedestrianGender.MALE,
    'female': unreal.PedestrianGender.FEMALE,
    'other': unreal.PedestrianGender.OTHER,
}

AGES = {
    'child': unreal.PedestrianAge.CHILD,
    'teenager': unreal.PedestrianAge.TEENAGER,
    'adult': unreal.PedestrianAge.ADULT,
    'elderly': unreal.PedestrianAge.ELDERLY,
}


def blueprint_class_path(path):
    """Accept either form and return the generated-class object path.

    A path copied from the Content Browser is the PACKAGE
    (/Game/.../BP_Walker_Foo); load_object needs the OBJECT
    (/Game/.../BP_Walker_Foo.BP_Walker_Foo_C). Appending '_C' to the package path -
    which is what the vehicle script does, assuming Package.Object input - silently
    yields an unloadable path, so both spellings are normalised here.
    """
    path = path.rstrip('/')
    if path.endswith('_C'):
        path = path[:-2]
    package, _, obj = path.rpartition('/')
    if '.' in obj:                       # already Package.Object
        return path + '_C'
    return '%s/%s.%s_C' % (package, obj, obj)


def parse_speed(text):
    values = [float(v) for v in text.replace(' ', '').split(',') if v]
    if len(values) != 3:
        raise ValueError('--speed takes three values: idle,walk,run')
    return values


def next_free_id(pedestrian_list):
    """Ids are 4-digit and dense, so the next one is max + 1."""
    used = []
    for entry in pedestrian_list:
        value = str(entry.get_editor_property('id'))
        if value.isdigit():
            used.append(int(value))
    return '%04d' % ((max(used) + 1) if used else 1)


argparser = argparse.ArgumentParser()
argparser.add_argument(
    '-w', '--walker_blueprint_path',
    metavar='W',
    default='',
    type=str,
    help='Path to the walker blueprint to register')
argparser.add_argument(
    '--id',
    metavar='I',
    default='',
    type=str,
    help='4-digit factory id (default: the next free one)')
argparser.add_argument(
    '--gender',
    default='other',
    choices=sorted(GENDERS),
    help='pedestrian gender attribute')
argparser.add_argument(
    '--age',
    default='adult',
    choices=sorted(AGES),
    help='pedestrian age attribute')
argparser.add_argument(
    '--speed',
    default='0.0,1.7,4.0',
    type=str,
    help='idle,walk,run in m/s')
argparser.add_argument(
    '--generation',
    default=3,
    type=int,
    help='pedestrian generation attribute')
argparser.add_argument(
    '--wheelchair',
    action='store_true',
    help='set can_use_wheelchair on the definition')
argparser.add_argument(
    '--array',
    default='Pedestrians',
    type=str,
    help='name of the factory member variable holding the pedestrian list')
argparser.add_argument(
    '--result',
    default='',
    type=str,
    help='optional path to write a JSON summary to')
args = argparser.parse_args()

result = {'ok': False, 'array': args.array}

# load walker blueprint and factory
walker_factory_path = WALKER_FACTORY_PATH + '.WalkerFactory_C'
walker_factory_class = unreal.load_object(None, walker_factory_path)
walker_factory_default_object = unreal.get_default_object(walker_factory_class)
walker_blueprint_path = blueprint_class_path(args.walker_blueprint_path)
walker_blueprint = unreal.load_object(None, walker_blueprint_path)

if walker_blueprint is None:
    raise RuntimeError('cannot load the walker blueprint %s' % walker_blueprint_path)

try:
    pedestrian_list = list(
        walker_factory_default_object.get_editor_property(args.array))
except Exception as exc:
    raise RuntimeError(
        "WalkerFactory has no member variable '%s' (%s). The pedestrian list must be "
        "a blueprint MEMBER variable, not a local inside GenerateDefinitions."
        % (args.array, exc))

count_before = len(pedestrian_list)
walker_id = args.id if args.id else next_free_id(pedestrian_list)

# generate the new field
new_pedestrian_parameters = unreal.PedestrianParameters()
new_pedestrian_parameters.id = walker_id
new_pedestrian_parameters.class_ = walker_blueprint
new_pedestrian_parameters.gender = GENDERS[args.gender]
new_pedestrian_parameters.age = AGES[args.age]
new_pedestrian_parameters.speed = parse_speed(args.speed)
new_pedestrian_parameters.generation = args.generation
new_pedestrian_parameters.can_use_wheel_chair = args.wheelchair

# a re-import of the same walker updates its entry instead of adding a second one
replaced = None
for index, entry in enumerate(pedestrian_list):
    existing = entry.get_editor_property('class_')
    if existing is not None and str(existing) == str(walker_blueprint):
        replaced = index
        # keep the id the walker already had
        walker_id = str(entry.get_editor_property('id'))
        new_pedestrian_parameters.id = walker_id
        break

if replaced is None:
    pedestrian_list.append(new_pedestrian_parameters)
    action = 'appended'
else:
    pedestrian_list[replaced] = new_pedestrian_parameters
    action = 'updated'

walker_factory_default_object.set_editor_property(args.array, pedestrian_list)

count_after = len(walker_factory_default_object.get_editor_property(args.array))
floor = count_before + (0 if replaced is not None else 1)
if count_after < floor:
    raise RuntimeError(
        'the %s array did not take the write (%d -> %d, expected at least %d); '
        'nothing was saved' % (args.array, count_before, count_after, floor))

# compile and save the factory blueprint, as pressing Compile then Save does
walker_factory_blueprint = unreal.load_asset(WALKER_FACTORY_PATH)
if hasattr(unreal, 'VehicleAuthoringLibrary'):
    saved = unreal.VehicleAuthoringLibrary.compile_and_save_blueprint(
        walker_factory_blueprint)
else:
    saved = unreal.EditorAssetLibrary.save_asset(WALKER_FACTORY_PATH, False)

verify = walker_factory_default_object.get_editor_property(args.array)
result.update({
    'ok': bool(saved) and len(verify) >= floor,
    'action': action,
    'id': walker_id,
    'blueprint_id': 'walker.pedestrian.' + walker_id.lower(),
    'walker_blueprint': walker_blueprint_path,
    'entries_before': count_before,
    'entries_after': len(verify),
    'saved': bool(saved),
})

print('%s walker.pedestrian.%s in %s.%s (%d -> %d entries)' % (
    action, walker_id, WALKER_FACTORY_PATH, args.array,
    count_before, len(verify)))

if args.result:
    with open(args.result, 'w') as handle:
        json.dump(result, handle, indent=2)
