"""Run Scenic against a NuRec proxy map without rendering; save actor/control telemetry.

Use a dedicated CARLA server: this command replaces its world.
python -O validate_behavior.py --usdz SCENE.usdz --scenario FILE.scenic --out RUN
"""
import argparse
import json
import math
from pathlib import Path
import random
import socket
import msgpack
import sys
import zipfile
import hashlib
import queue

import carla
import numpy as np
import scenic
from scenic.core.simulators import Simulation, Simulator
from scenic.domains.driving.simulators import DrivingSimulator
from scenic.simulators.carla.simulator import CarlaSimulator, CarlaSimulation

HERE = Path(__file__).resolve().parent
sys.path[:0] = [str(HERE), str(HERE.parent), str(HERE.parent.parent)]
from projection_functions import get_t_rig_enu_from_ecef
from utils import mat_to_carla_transform
import scenic_xodr
from prop_grounding import ground_prop, snap_ramp_vehicle, DRIVABLE_LANES


def main():
    ap = argparse.ArgumentParser(description=__doc__)
    ap.add_argument('--usdz', required=True)
    ap.add_argument('--scenario', required=True)
    ap.add_argument('--out', required=True)
    ap.add_argument('--port', type=int, default=4690)
    ap.add_argument('--seed', type=int, default=0)
    ap.add_argument('--seconds', type=float, default=25)
    ap.add_argument('--fps', type=float, default=20)
    ap.add_argument('--param', action='append', default=[])
    ap.add_argument('--sample-only', action='store_true')
    ap.add_argument('--preview', action='store_true',help='Capture fast raster ego-camera frames for behavior review')
    a = ap.parse_args()
    out = Path(a.out); out.mkdir(parents=True, exist_ok=True)
    with zipfile.ZipFile(a.usdz) as z:
        xodr = z.read('map.xodr').decode()
        rig = json.loads(z.read('rig_trajectories.json'))
    T = get_t_rig_enu_from_ecef(np.array(rig['T_world_base']), xodr)
    trajectory = rig['rig_trajectories'][0]
    poses = [np.asarray(p).reshape(4, 4) for p in trajectory['T_rig_worlds']]
    start = mat_to_carla_transform(T @ poses[0])
    end = mat_to_carla_transform(T @ poses[-1])
    clean, report = scenic_xodr.sanitize(xodr)
    map_path = out / 'map_scenic.xodr'; map_path.write_text(clean)
    client = carla.Client('localhost', a.port); client.set_timeout(180)
    # Match hybrid_scenic's bare proxy road even on a fresh server.
    # Otherwise procedural furniture can obstruct the reconstructed ramp.
    with socket.create_connection(('localhost', a.port), timeout=10) as sock:
        sock.sendall(msgpack.packb([0, 1, 'console_command', [[False],
            'carla.OpenDrive.StreetFurniture 0']], use_bin_type=True))
        sock.recv(1 << 16)
    world = client.generate_opendrive_world(xodr, carla.OpendriveGenerationParameters(
        vertex_distance=2, max_road_length=500, wall_height=0, additional_width=.6,
        smooth_junctions=True, enable_mesh_visibility=a.preview))
    original = world.get_settings()
    tm = client.get_trafficmanager(a.port + 10)
    telemetry = []; collisions = []; sensors = []; preview_images=queue.Queue()
    try:
        settings = world.get_settings(); settings.synchronous_mode = True
        settings.fixed_delta_seconds = 1 / a.fps
        settings.no_rendering_mode = not a.preview
        settings.substepping = True; settings.max_substep_delta_time = .01; settings.max_substeps = 10
        world.apply_settings(settings); tm.set_synchronous_mode(True)
        wp = world.get_map().get_waypoint(start.location, lane_type=DRIVABLE_LANES)
        params = dict(map=str(map_path.resolve()), carla_map=None, timestep=1/a.fps,
            render=0, weather=None, use2DMap=True, snapToGroundDefault=True,
            ego_x=wp.transform.location.x, ego_y=-wp.transform.location.y,
            ego_heading=-math.radians(wp.transform.rotation.yaw + 90),
            clip_end_x=end.location.x, clip_end_y=-end.location.y,
            clip_length_m=sum(np.linalg.norm(q[:3,3]-p[:3,3]) for p,q in zip(poses,poses[1:])))
        for kv in a.param:
            k,v = kv.split('=',1)
            try: v=json.loads(v)
            except json.JSONDecodeError: pass
            params[k]=v
        random.seed(a.seed); np.random.seed(a.seed)
        (out/'params.json').write_text(json.dumps(params, indent=2))
        scenario = scenic.scenarioFromFile(a.scenario, params=params, mode2D=True)
        scene,_ = scenario.generate(maxIterations=10 if a.sample_only else 2000, verbosity=2 if a.sample_only else 0)
        if a.sample_only:
            print('Sampled', len(scene.objects), 'objects')
            return

        class CapturedCollision(Exception):
            pass

        class PhysicsSimulation(CarlaSimulation):
            def createObjectInSimulator(self, obj):
                # Scenic uses +0.5 m for every actor, which embeds tall walker
                # capsules in the outer road shoulder. Spawn above the actual
                # collision surface, then let the existing warm-up settle feet.
                from unittest.mock import patch
                from scenic.simulators.carla.simulator import utils
                original_location = utils.scenicToCarlaLocation
                def grounded_location(pos, world=None, blueprint=None, snapToGround=False):
                    loc = original_location(pos, world=world, blueprint=blueprint, snapToGround=snapToGround)
                    if snapToGround and blueprint and blueprint.startswith('walker.'):
                        hits = world.cast_ray(carla.Location(loc.x,loc.y,loc.z+3),carla.Location(loc.x,loc.y,loc.z-3))
                        surfaces = [h.location.z for h in hits if h.label in (carla.CityObjectLabel.Roads,carla.CityObjectLabel.Sidewalks,carla.CityObjectLabel.Terrain)]
                        if not surfaces: raise ValueError('No collision surface under walker spawn')
                        loc.z = max(surfaces) + 1.05
                    return loc
                with patch.object(utils, 'scenicToCarlaLocation', grounded_location):
                    super().createObjectInSimulator(obj)
            def setup(self):
                Simulation.setup(self)
                # Actor transform snapshots are populated by the first tick.
                world.tick()
                for obj in self.objects:
                    if obj.snapToGround: snap_ramp_vehicle(world, obj.carlaActor)
                world.tick()
                grounding = [result for obj in self.objects
                             if (result := ground_prop(world, obj.carlaActor)) is not None]
                (out/'prop_grounding.json').write_text(json.dumps(grounding, indent=2))
                world.tick()
                for obj in self.objects:
                    if isinstance(obj.carlaActor, carla.Vehicle):
                        obj.carlaActor.apply_control(carla.VehicleControl(manual_gear_shift=False, brake=1))
                for _ in range(30): world.tick()
                if a.preview:
                    bp=world.get_blueprint_library().find('sensor.camera.rgb')
                    for key,value in {'image_size_x':'640','image_size_y':'360','fov':'90','sensor_tick':'0','motion_blur_intensity':'0'}.items():
                        if bp.has_attribute(key):bp.set_attribute(key,value)
                    camera=world.spawn_actor(bp,carla.Transform(carla.Location(x=1.3,z=1.45)),attach_to=self.objects[0].carlaActor)
                    camera.listen(preview_images.put);sensors.append(camera)
                # Identical settle time with and without RGB capture.
                for _ in range(8):world.tick()
                (out/'setup_actors.json').write_text(json.dumps([
                    dict(role=obj.rolename, transform=str(obj.carlaActor.get_transform()))
                    for obj in self.objects], indent=2))
                for obj in self.objects:
                    actor = obj.carlaActor
                    if isinstance(actor, carla.Vehicle):
                        actor.apply_control(carla.VehicleControl(manual_gear_shift=False))
                        v = actor.get_transform().get_forward_vector() * float(obj.speed or 0)
                        actor.set_target_velocity(v)
                        sensor=world.spawn_actor(self.blueprintLib.find('sensor.other.collision'),carla.Transform(),attach_to=actor)
                        sensor.listen(lambda event, role=obj.rolename or actor.type_id: collisions.append(
                            dict(frame=event.frame, actor=role, other=event.other_actor.type_id,
                                 impulse=event.normal_impulse.length())))
                        sensors.append(sensor)
            def executeActions(self, allActions):
                # Scenic's CARLA adapter sends each actor's control asynchronously;
                # a tick can overtake the final update. Submit one synchronous
                # batch before advancing, preserving exactly one world tick.
                if collisions: raise CapturedCollision()
                super(CarlaSimulation, self).executeActions(allActions)
                commands = []
                for obj in self.agents:
                    ctrl = obj._control
                    if ctrl is not None:
                        command = carla.command.ApplyWalkerControl if isinstance(ctrl, carla.WalkerControl) else carla.command.ApplyVehicleControl
                        commands.append(command(obj.carlaActor.id, ctrl))
                        obj._control = None
                if commands:
                    replies = client.apply_batch_sync(commands, False)
                    errors = [reply.error for reply in replies if reply.error]
                    if errors and collisions: raise CapturedCollision()
                    if errors: raise RuntimeError(f'Control batch failed: {errors}')
            def step(self):
                self.current_frame=world.tick()
                actors=[]
                for obj in self.objects:
                    actor=obj.carlaActor; tf=actor.get_transform(); loc=tf.location
                    row=dict(role=obj.rolename or actor.type_id, blueprint=actor.type_id,
                             x=loc.x,y=loc.y,z=loc.z,yaw=tf.rotation.yaw,pitch=tf.rotation.pitch,roll=tf.rotation.roll,speed=actor.get_velocity().length())
                    row['id'] = actor.id
                    row['footprint'] = [[v.x, v.y] for v in actor.bounding_box.get_world_vertices(tf)]
                    if isinstance(actor,carla.Vehicle):
                        ctl=actor.get_control(); row.update(throttle=ctl.throttle,brake=ctl.brake,steer=ctl.steer)
                        lane = self.map.get_waypoint(loc, lane_type=DRIVABLE_LANES)
                        row['lane'] = [lane.road_id, lane.lane_id] if lane else None
                    actors.append(row)
                telemetry.append(dict(frame=self.current_frame,t=(len(telemetry)+1)/a.fps,actors=actors))
            def destroy(self):
                if a.preview:
                    for _ in range(3):world.tick()
                for sensor in sensors:
                    if sensor.is_alive: sensor.stop(); sensor.destroy()
                sensors.clear()
                super().destroy()

        class PhysicsSimulator(CarlaSimulator):
            def __init__(self):
                DrivingSimulator.__init__(self)
            def createSimulation(self, scene, *, timestep, **kwargs):
                return PhysicsSimulation(scene,client,tm,False,'',1,timestep=1/a.fps,**kwargs)
            def destroy(self): Simulator.destroy(self)

        simulator=PhysicsSimulator()
        stopped_on_collision = False
        try: sim=simulator.simulate(scene,maxSteps=int(a.seconds*a.fps),maxIterations=1,verbosity=1)
        except CapturedCollision:
            sim = None
            stopped_on_collision = True
        finally: simulator.destroy()
        result=dict(scenario=str(Path(a.scenario).resolve()),usdz=str(Path(a.usdz).resolve()),seed=a.seed,
                    scenario_sha256=hashlib.sha256(Path(a.scenario).read_bytes()).hexdigest(),server_version=client.get_server_version(),
                    params=params,map_report=report,termination='first collision (diagnostic stop)' if stopped_on_collision else str(sim.result.terminationReason) if sim else 'rejected',
                    collisions=collisions,frames=telemetry)
        if 'late_crossing_helpers' in sys.modules:
            result['events']=list(sys.modules['late_crossing_helpers'].EVENTS)
        (out/'behavior.json').write_text(json.dumps(result,indent=2))
        if a.preview:
            from fast_preview import save_preview
            images={}
            while not preview_images.empty():
                image=preview_images.get();images[image.frame]=image
            save_preview(result,images,out,a.fps)
        print(json.dumps(dict(frames=len(telemetry),collisions=len(collisions),termination=result['termination'])))
        if sim is None and not stopped_on_collision: raise RuntimeError('Scenic rejected the run')
    finally:
        for sensor in sensors:
            if sensor.is_alive: sensor.stop(); sensor.destroy()
        world.apply_settings(original); tm.set_synchronous_mode(False)
        if telemetry and not (out/'behavior.json').exists():
            (out/'partial.json').write_text(json.dumps(dict(frames=telemetry,collisions=collisions)))


if __name__=='__main__': main()
