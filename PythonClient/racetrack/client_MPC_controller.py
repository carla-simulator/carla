from __future__ import print_function

import argparse
import logging
import random
import time
import pandas as pd
import numpy as np
from scipy import spatial
from scipy.interpolate import splprep, splev
import matplotlib.pyplot as plt

import sys
sys.path.append('..')
from carla.client import make_carla_client
from carla.sensor import Camera, Lidar
from carla.settings import CarlaSettings
from carla.tcp import TCPConnectionError
from carla.util import print_over_same_line

from scipy.optimize import minimize
import sympy as sym
sym.init_printing()


norm = np.linalg.norm


STATE_VARS = ('x', 'y', 'v', 'ψ', 'cte', 'eψ')
STEPS_AHEAD = 2
dt = 0.01
CTE_COEFF = 4000
EPSI_COEFF = 4000
SPEED_COEFF = 2
ACC_COEFF = 1
STEER_COEFF = 1
Lf = 2.67
POLY_DEGREE = 3
EVALUATOR = 'math'


def create_array_of_symbols(str_symbol, N):
    return sym.symbols('{symbol}0:{N}'.format(symbol=str_symbol, N=N))


def generate_fun(symb_fun, vars, poly):
    '''This function generates a function of the form `fun(x, *args)` because
    that's what the scipy `minimize` API expects (if we don't want to minimize
    over certain variables, we pass them as `args`)
    '''
    func = sym.lambdify(vars+poly, symb_fun, EVALUATOR)
    dupa = lambda x, *args: func(*np.r_[x, args])
    return dupa


def generate_grad(symb_fun, vars, poly):
    cost_grad_funcs = [
        generate_fun(symb_fun.diff(var), vars, poly)
        for var in vars
    ]
    return lambda x, *args: [
        grad_func(*np.r_[x, args]) for grad_func in cost_grad_funcs
    ]


class EqualityConstraints(object):
    def __init__(self, N):
        self.dict = {}
        for symbol in STATE_VARS:
            self.dict[symbol] = N*[None]

    def __getitem__(self, key):
        return self.dict[key]

    def __setitem__(self, key, value):
        self.dict[key] = value


class State(object):
    def __init__(self):
        self.dict = {}
        for symbol in STATE_VARS:
            self.dict[symbol] = N*[None]

    def __getitem__(self, key):
        return self.dict[key]

    def __setitem__(self, key, value):
        self.dict[key] = value



def run_carla_client(args):
    number_of_episodes = 10
    frames_per_episode = 10000
    spline_points = 10000

    track_DF = pd.read_csv('racetrack.txt', header=None)
    # The track data are rescaled by 100x with relation to Carla measurements
    track_DF = track_DF / 100

    pts_2D = track_DF.loc[:, [0, 1]].values
    tck, u = splprep(pts_2D.T, u=None, s=2.0, per=1, k=3)
    u_new = np.linspace(u.min(), u.max(), spline_points)
    x_new, y_new = splev(u_new, tck, der=0)
    pts_2D = np.c_[x_new, y_new]

    prev_speed = np.nan
    prev_prev_speed = np.nan
    curr_speed = np.nan

    prev_prop = np.nan
    prev_prev_prop = np.nan
    curr_prop = np.nan
    deriv_list = []
    deriv_len = 5

    steer = 0
    throttle = 0.5
    target_speed = args.target_speed

    kp = args.kp
    kd = args.kd

    depth_array = None

    # Polynomial coefficients will also be symbolic variables
    poly = create_array_of_symbols('poly', POLY_DEGREE+1)

    # Initialize the initial state
    x_init = sym.symbols('x_init')
    y_init = sym.symbols('y_init')
    ψ_init = sym.symbols('ψ_init')
    v_init = sym.symbols('v_init')
    cte_init = sym.symbols('cte_init')
    eψ_init = sym.symbols('eψ_init')

    # State variables
    x = create_array_of_symbols('x', STEPS_AHEAD)
    y = create_array_of_symbols('y', STEPS_AHEAD)
    ψ = create_array_of_symbols('ψ', STEPS_AHEAD)
    v = create_array_of_symbols('v', STEPS_AHEAD)
    cte = create_array_of_symbols('cte', STEPS_AHEAD)
    eψ = create_array_of_symbols('eψ', STEPS_AHEAD)

    # Actuators
    a = create_array_of_symbols('a', STEPS_AHEAD)
    δ = create_array_of_symbols('δ', STEPS_AHEAD)

    vars = (
        # Initializing variables
        x_init, y_init, ψ_init, v_init, cte_init, eψ_init,

        # Symbolic arrays (but NOT actuators)
        *x, *y, *ψ, *v, *cte, *eψ,

        # Symbolic arrays (actuators)
        *a, *δ,
    )

    cost = 0
    for i in range(STEPS_AHEAD):
        cost += (
        # Reference state penalties
        CTE_COEFF * cte[i]**2 +
        EPSI_COEFF * eψ[i]**2 +
        SPEED_COEFF * (v[i] - target_speed)**2 +

        # Actuators penalties
        ACC_COEFF * a[i] +
        STEER_COEFF * δ[i]
    )

    cost_grad = [cost.diff(var) for var in vars]

    # Initialize constraints
    eq_constr = EqualityConstraints(STEPS_AHEAD)
    eq_constr['x'][0] = x[0] - x_init
    eq_constr['y'][0] = y[0] - y_init
    eq_constr['ψ'][0] = ψ[0] - ψ_init
    eq_constr['v'][0] = v[0] - v_init
    eq_constr['cte'][0] = cte[0] - cte_init
    eq_constr['eψ'][0] = eψ[0] - eψ_init

    for t in range(1, STEPS_AHEAD):
        curve = sum(poly[i] * x[t-1]**i for i in range(len(poly)))
        # The desired ψ is equal to the derivative of the polynomial curve at
        #  point x[t-1]
        ψdes = sum(poly[i] * i*x[t-1]**(i-1) for i in range(1, len(poly)))

        eq_constr['x'][t] = x[t] - (x[t-1] + v[t-1] * sym.cos(ψ[t-1]) * dt)
        eq_constr['y'][t] = y[t] - (y[t-1] + v[t-1] * sym.sin(ψ[t-1]) * dt)
        eq_constr['ψ'][t] = ψ[t] - (ψ[t-1] - v[t-1] * δ[t-1] / Lf * dt)
        eq_constr['v'][t] = v[t] - (v[t-1] + a[t-1] * dt)
        eq_constr['cte'][t] = cte[t] - (curve - y[t-1] + v[t-1] * sym.sin(eψ[t-1]) * dt)
        eq_constr['eψ'][t] = eψ[t] - (ψ[t-1] - ψdes - v[t-1] * δ[t-1] / Lf * dt)

    # Generate actual functions from
    cost_func = generate_fun(cost, vars, poly)
    cost_grad_func = generate_grad(cost, vars, poly)

    constr_funcs = []
    for symbol in STATE_VARS:
        for t in range(STEPS_AHEAD):
            func = generate_fun(eq_constr[symbol][t], vars, poly)
            grad_func = generate_grad(eq_constr[symbol][t], vars, poly)
            constr_funcs.append(
                {'type': 'eq', 'fun': func, 'jac': grad_func, 'args': None},
            )

    init_values = np.array([
        1, 1, 1, 60, 1, 1.2,
        1.1, 1.1,
        0.9, 1.0,
        59, 58,
        0.9, 1.1,
        1.1, 1.1,
        0.0, 0.0,
        0.5, 0.5,
        0.0, 0.0,
    ])
    args = (1, 2, 3, 4)

    bounds = 18*[(None, None)] + [(-1, 1), (-1, 1), (-4, 4), (-4, 4)]

    start_time = time.time()
    for i in range(1000):
        # TODO: this is a bit retarded, but hey -- that's scipy API's fault ;)
        for constr_func in constr_funcs:
            constr_func['args'] = args
        minimize(
            fun=cost_func,
            x0=init_values,
            args=args,
            jac=cost_grad_func,
            bounds=bounds,
            constraints=constr_funcs,
            method='SLSQP'
        )
    print('Took {:.4f}s'.format((time.time() - start_time) / 1000))
    import ipdb; ipdb.set_trace()


    # We assume the CARLA server is already waiting for a client to connect at
    # host:port. To create a connection we can use the `make_carla_client`
    # context manager, it creates a CARLA client object and starts the
    # connection. It will throw an exception if something goes wrong. The
    # context manager makes sure the connection is always cleaned up on exit.
    with make_carla_client(args.host, args.port) as client:
        print('CarlaClient connected')
        for episode in range(0, number_of_episodes):
            # Start a new episode.
            storage = np.random.rand(150, 200, frames_per_episode).astype(np.float16)
            stream = open('log{}.txt'.format(episode), 'w')
            stream.write('frame,steer,throttle,speed\n')

            if args.settings_filepath is None:

                # Create a CarlaSettings object. This object is a wrapper around
                # the CarlaSettings.ini file. Here we set the configuration we
                # want for the new episode.
                settings = CarlaSettings()
                settings.set(
                    SynchronousMode=True,
                    SendNonPlayerAgentsInfo=True,
                    NumberOfVehicles=0,
                    NumberOfPedestrians=40,
                    WeatherId=random.choice([1]),
                    QualityLevel=args.quality_level)
                settings.randomize_seeds()

                # Now we want to add a couple of cameras to the player vehicle.
                # We will collect the images produced by these cameras every
                # frame.

                # The default camera captures RGB images of the scene.
                camera0 = Camera('CameraRGB')
                # Set image resolution in pixels.
                camera0.set_image_size(800, 600)
                # Set its position relative to the car in meters.
                camera0.set_position(0.30, 0, 1.30)
                settings.add_sensor(camera0)

                # Let's add another camera producing ground-truth depth.
                camera1 = Camera('CameraDepth', PostProcessing='Depth')
                camera1.set_image_size(200, 150)
                camera1.set_position(2.30, 0, 1.30)
                settings.add_sensor(camera1)

            else:

                # Alternatively, we can load these settings from a file.
                with open(args.settings_filepath, 'r') as fp:
                    settings = fp.read()

            # Now we load these settings into the server. The server replies
            # with a scene description containing the available start spots for
            # the player. Here we can provide a CarlaSettings object or a
            # CarlaSettings.ini file as string.
            scene = client.load_settings(settings)

            # Choose one player start at random.
            number_of_player_starts = len(scene.player_start_spots)
            player_start = random.randint(0, max(0, number_of_player_starts - 1))

            # Notify the server that we want to start the episode at the
            # player_start index. This function blocks until the server is ready
            # to start the episode.
            print('Starting new episode...')
            client.start_episode(player_start)

            # Iterate every frame in the episode.
            for frame in range(0, frames_per_episode):

                # Read the data produced by the server this frame.
                measurements, sensor_data = client.read_data()

                # Print some of the measurements.
                print_measurements(measurements)

                # Get current location
                location = np.array([
                    measurements.player_measurements.transform.location.x,
                    measurements.player_measurements.transform.location.y,
                ])

                # Get closest point's distance
                dists = norm(pts_2D - location, axis=1)
                which_closest = np.argmin(dists)
                closest, next = pts_2D[which_closest], pts_2D[which_closest+1]
                road_direction = next - closest
                perpendicular = np.array([
                    -road_direction[1],
                    road_direction[0],
                ])
                steer_direction = (location-closest).dot(perpendicular)
                prev_prev_prop = prev_prop
                prev_prop = curr_prop
                curr_prop = np.sign(steer_direction) * dists[which_closest]

                if any(pd.isnull([prev_prev_prop, prev_prop, curr_prop])):
                    deriv = 0
                else:
                    deriv = 0.5 * (curr_prop - prev_prev_prop)
                    deriv_list.append(deriv)
                    if len(deriv_list) > deriv_len:
                        deriv_list = deriv_list[-deriv_len:]
                    deriv = np.mean(deriv_list)

                prev_prev_speed = prev_speed
                prev_speed = curr_speed
                curr_speed = measurements.player_measurements.forward_speed * 3.6
                # TODO: find a better way of keeping the speed constant
                throttle = np.clip(
                    throttle - 0.1 * (curr_speed-target_speed),
                    0.25,
                    1.0
                )

                steer = -kp * curr_prop - kd * deriv + np.random.uniform(-0.05, 0.05)

                print(
                    ' steer_direction = {:.2f}'
                    ' prop = {:.2f}'
                    ' deriv = {:.5f}'
                    ' throttle = {:.2f}'
                    ' curr_speed = {:.2f}'
                    ' steer = {:.2f}'
                    .format(steer_direction, curr_prop, deriv, throttle, curr_speed, steer)
                )

                client.send_control(
                    steer=steer,
                    throttle=throttle,
                    brake=0.0,
                    hand_brake=False,
                    reverse=False)

                depth_array = np.log(sensor_data['CameraDepth'].data).astype('float16')
                storage[..., frame] = depth_array
                stream.write(
                    '{},{},{},{}\n'
                    .format(frame, steer, throttle, curr_speed)
                )

            np.save('depth_data{}.npy'.format(episode), storage)
            stream.close()


def print_measurements(measurements):
    number_of_agents = len(measurements.non_player_agents)
    player_measurements = measurements.player_measurements
    message = 'Vehicle at ({pos_x:.1f}, {pos_y:.1f}), '
    message += '{speed:.0f} km/h, '
    message = message.format(
        pos_x=player_measurements.transform.location.x,
        pos_y=player_measurements.transform.location.y,
        speed=player_measurements.forward_speed * 3.6, # m/s -> km/h
    )
    print_over_same_line(message)


def main():
    argparser = argparse.ArgumentParser(description=__doc__)
    argparser.add_argument(
        '-v', '--verbose',
        action='store_true',
        dest='debug',
        help='print debug information')
    argparser.add_argument(
        '--host',
        metavar='H',
        default='localhost',
        help='IP of the host server (default: localhost)')
    argparser.add_argument(
        '-p', '--port',
        metavar='P',
        default=2000,
        type=int,
        help='TCP port to listen to (default: 2000)')
    argparser.add_argument(
        '-q', '--quality-level',
        choices=['Low', 'Epic'],
        type=lambda s: s.title(),
        default='Epic',
        help='graphics quality level, a lower level makes the simulation run considerably faster.')
    argparser.add_argument(
        '-c', '--carla-settings',
        metavar='PATH',
        dest='settings_filepath',
        default=None,
        help='Path to a "CarlaSettings.ini" file')

    argparser.add_argument(
        '-kp', '--k-prop',
        default=0.6,
        type=float,
        dest='kp',
        help='PID`s controller "proportion" coefficient')
    argparser.add_argument(
        '-kd', '--k-deriv',
        default=6.5,
        type=float,
        dest='kd',
        help='PID`s controller "derivative" coefficient')
    argparser.add_argument(
        '-s', '--speed',
        default=45,
        type=float,
        dest='target_speed',
        help='Target speed')

    args = argparser.parse_args()

    log_level = logging.DEBUG if args.debug else logging.INFO
    logging.basicConfig(format='%(levelname)s: %(message)s', level=log_level)

    logging.info('listening to server %s:%s', args.host, args.port)

    args.out_filename_format = '_out/episode_{:0>4d}/{:s}/{:0>6d}'

    while True:
        try:

            run_carla_client(args)

            print('Done.')
            return

        except TCPConnectionError as error:
            logging.error(error)
            time.sleep(1)


if __name__ == '__main__':

    try:
        main()
    except KeyboardInterrupt:
        print('\nCancelled by user. Bye!')
