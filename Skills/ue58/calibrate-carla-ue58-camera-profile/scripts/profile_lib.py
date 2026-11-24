"""Camera-profile files and captures, shared by calibrate.py and capture.py.

A profile is Content/Carla/Config/PostProcess/<Name>.json: an FPostProcessSettings
dump under "settings". The server reads it from disk every time a camera with
post_process_profile=<Name> spawns, so a new or edited file takes effect on the
next spawned camera, with no restart.
"""
import json
import os
import random
import time

HERE = os.path.dirname(os.path.abspath(__file__))
BASE_DIR = os.path.join(HERE, '..', 'references', 'base')


# ---------------------------------------------------------------- profile files

def content_dir(root):
    """Content dir of a ue58-dev tree (Unreal/CarlaUnreal/Content) or of a package (CarlaUnreal/Content)."""
    for c in (os.path.join(root, 'Unreal', 'CarlaUnreal', 'Content'), os.path.join(root, 'CarlaUnreal', 'Content')):
        if os.path.isdir(c):
            return c
    raise SystemExit('no CarlaUnreal Content directory under %s (pass the tree or package root)' % root)


def profile_dir(root):
    d = os.path.join(content_dir(root), 'Carla', 'Config', 'PostProcess')
    os.makedirs(d, exist_ok=True)
    return d


def load_base(root, name):
    """A shipped profile: from the tree if it has it, else the copy in references/base (packages keep theirs in the .pak)."""
    for p in (os.path.join(profile_dir(root), name + '.json'), os.path.join(BASE_DIR, name + '.json')):
        if os.path.isfile(p):
            return json.load(open(p))
    raise SystemExit('base profile %s not found in the tree nor in references/base' % name)


def set_field(settings, path, value):
    """Set settings[path] (case-insensitive, 'colorGain.x' for a component) and switch its bOverride_ on."""
    low = {k.lower(): k for k in settings}
    field, _, comp = path.partition('.')
    k = low.get(field.lower())
    if k is None:
        raise KeyError('no post-process field %r' % field)
    if comp:
        settings[k] = dict(settings[k])
        settings[k][comp] = value
    else:
        settings[k] = value
    ov = low.get('boverride_' + field.lower())
    if ov:
        settings[ov] = True


def get_field(settings, path):
    low = {k.lower(): k for k in settings}
    field, _, comp = path.partition('.')
    v = settings[low[field.lower()]]
    return v[comp] if comp else v


def write_profile(root, name, doc):
    p = os.path.join(profile_dir(root), name + '.json')
    with open(p, 'w') as f:
        json.dump(doc, f, indent='\t')
    return p


# ---------------------------------------------------------------- captures

def pick_views(world, n, seed=7, height=1.7):
    """n street-level views spread over the map: spawn points (on a lane, facing along it) raised to eye height."""
    import carla
    sps = world.get_map().get_spawn_points()
    rnd = random.Random(seed)
    rnd.shuffle(sps)
    views = []
    for sp in sps:
        if all(sp.location.distance(v.location) > 60 for v in views):
            views.append(carla.Transform(carla.Location(sp.location.x, sp.location.y, sp.location.z + height),
                                         carla.Rotation(pitch=2.0, yaw=sp.rotation.yaw)))
        if len(views) == n:
            break
    return views


def views_from_file(path):
    import carla
    out = []
    for v in json.load(open(path)):
        out.append(carla.Transform(carla.Location(*v['location']), carla.Rotation(*v.get('rotation', [0, 0, 0]))))
    return out


class Session:
    """Synchronous mode for the duration of the captures; restores the world settings on close."""

    def __init__(self, host='localhost', port=2000, weather=None, delta=0.05, timeout=60.0):
        import carla
        self.carla = carla
        self.client = carla.Client(host, port)
        self.client.set_timeout(timeout)
        self.world = self.client.get_world()
        self.orig = self.world.get_settings()
        s = self.world.get_settings()
        s.synchronous_mode = True
        s.fixed_delta_seconds = delta
        self.world.apply_settings(s)
        if weather:
            w = getattr(carla.WeatherParameters, weather, None)
            if w is None:
                raise SystemExit('unknown weather preset %s' % weather)
            self.world.set_weather(w)
        self.world.tick()

    def capture(self, profile, views, out_dir, width=1440, height=1080, fov=50.0, warmup=60, prefix='view'):
        """Spawn one camera with the profile, visit each view, let auto exposure settle, save one PNG per view."""
        import queue
        lib = self.world.get_blueprint_library()
        bp = lib.find('sensor.camera.rgb')
        if not bp.has_attribute('post_process_profile'):
            raise SystemExit('this server has no post_process_profile attribute (needs CARLA 1.0 / ue58-dev)')
        for k, v in dict(post_process_profile=profile, image_size_x=width, image_size_y=height, fov=fov).items():
            bp.set_attribute(k, str(v))
        os.makedirs(out_dir, exist_ok=True)
        cam = self.world.spawn_actor(bp, views[0])
        q = queue.Queue()
        cam.listen(q.put)
        files = []
        try:
            for i, v in enumerate(views):
                cam.set_transform(v)
                for _ in range(warmup):
                    self.world.tick()
                frame = self.world.tick()
                img = None
                deadline = time.time() + 30
                while time.time() < deadline:
                    img = q.get(timeout=30)
                    if img.frame >= frame:
                        break
                f = os.path.join(out_dir, '%s_%02d.png' % (prefix, i))
                img.save_to_disk(f)
                files.append(f)
                while not q.empty():
                    q.get_nowait()
        finally:
            cam.stop()
            cam.destroy()
            self.world.tick()
        return files

    def close(self):
        self.world.apply_settings(self.orig)
