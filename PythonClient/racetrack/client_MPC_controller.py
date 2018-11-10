from __future__ import print_function

import argparse
import logging
import random
import time
import pandas as pd
import numpy as np

from scipy.optimize import minimize
from scipy.interpolate import splprep, splev
import sympy as sym
from sympy.tensor.array import derive_by_array
sym.init_printing()

import sys
sys.path.append('..')
from carla.client import make_carla_client
from carla.sensor import Camera, Lidar
from carla.settings import CarlaSettings
from carla.tcp import TCPConnectionError
from carla.util import print_over_same_line

from utils import plot_points_with_numbers, clip_throttle


norm = np.linalg.norm


STATE_VARS = ('x', 'y', 'v', 'ψ', 'cte', 'eψ')

STEPS_AHEAD = 10
dt = 0.1

# Cost function coefficients
CTE_COEFF = 1000
EPSI_COEFF = 1000
SPEED_COEFF = 3  # 2
ACC_COEFF = 10  # 1
STEER_COEFF = 1  # 1
CONSEC_ACC_COEFF = 15
CONSEC_STEER_COEFF = 15
STEER_BOUND = 4

# Front wheel L
Lf = 2.67

# How the polynomial fitting the desired curve is fitted
STEPS_POLY = 20
POLY_DEGREE = 3

# Lambdify and minimize stuff
EVALUATOR = 'numpy'
LAT = 0.00
TOLERANCE = 1

IMAGE_SIZE = (150, 200)


def create_array_of_symbols(str_symbol, N):
    return sym.symbols('{symbol}0:{N}'.format(symbol=str_symbol, N=N))


def generate_fun(symb_fun, vars_, init, poly):
    '''This function generates a function of the form `fun(x, *args)` because
    that's what the scipy `minimize` API expects (if we don't want to minimize
    over certain variables, we pass them as `args`)
    '''
    args = init + poly
    return sym.lambdify((vars_, *args), symb_fun, EVALUATOR)
    # Equivalent to (but faster than):
    # func = sym.lambdify(vars_+init+poly, symb_fun, EVALUATOR)
    # return lambda x, *args: func(*np.r_[x, args])


def generate_grad(symb_fun, vars_, init, poly):
    args = init + poly
    return sym.lambdify(
        (vars_, *args),
        derive_by_array(symb_fun, vars_+args)[:len(vars_)]
    )
    # Equivalent to (but faster than):
    # cost_grad_funcs = [
    #     generate_fun(symb_fun.diff(var), vars_, init, poly)
    #     for var in vars_
    # ]
    # return lambda x, *args: [
    #     grad_func(np.r_[x, args]) for grad_func in cost_grad_funcs
    # ]


class EqualityConstraints(object):
    def __init__(self, N):
        self.dict = {}
        for symbol in STATE_VARS:
            self.dict[symbol] = N*[None]

    def __getitem__(self, key):
        return self.dict[key]

    def __setitem__(self, key, value):
        self.dict[key] = value


def get_func_constraints_and_bounds(target_speed):
    # Polynomial coefficients will also be symbolic variables
    poly = create_array_of_symbols('poly', POLY_DEGREE+1)

    # Initialize the initial state
    x_init = sym.symbols('x_init')
    y_init = sym.symbols('y_init')
    ψ_init = sym.symbols('ψ_init')
    v_init = sym.symbols('v_init')
    cte_init = sym.symbols('cte_init')
    eψ_init = sym.symbols('eψ_init')

    init = (x_init, y_init, ψ_init, v_init, cte_init, eψ_init)

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

    vars_ = (
        # Symbolic arrays (but NOT actuators)
        *x, *y, *ψ, *v, *cte, *eψ,

        # Symbolic arrays (actuators)
        *a, *δ,
    )

    cost = 0
    for t in range(STEPS_AHEAD):
        cost += (
            # Reference state penalties
            CTE_COEFF * cte[t]**2
            + EPSI_COEFF * eψ[t]**2 +
            + SPEED_COEFF * (v[t] - target_speed)**2
            #
            # # Actuators penalties
            + ACC_COEFF * a[t]**2
            + STEER_COEFF * δ[t]**2
        )

    # Penalty for differences in consecutive actuators
    for t in range(STEPS_AHEAD-1):
        cost += (
            CONSEC_ACC_COEFF * (a[t+1] - a[t])**2
            + CONSEC_STEER_COEFF * (δ[t+1] - δ[t])**2
        )

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
    cost_func = generate_fun(cost, vars_, init, poly)
    cost_grad_func = generate_grad(cost, vars_, init, poly)

    constr_funcs = []
    for symbol in STATE_VARS:
        for t in range(STEPS_AHEAD):
            func = generate_fun(eq_constr[symbol][t], vars_, init, poly)
            grad_func = generate_grad(eq_constr[symbol][t], vars_, init, poly)
            constr_funcs.append(
                {'type': 'eq', 'fun': func, 'jac': grad_func, 'args': None},
            )

    return cost_func, cost_grad_func, constr_funcs


def minimize_cost(cost_func, cost_grad_func, constr_funcs, bounds, x0, init):
    # TODO: this is a bit retarded, but hey -- that's scipy API's fault ;)
    for constr_func in constr_funcs:
        constr_func['args'] = init

    return minimize(
        fun=cost_func,
        x0=x0,
        args=init,
        jac=cost_grad_func,
        bounds=bounds,
        constraints=constr_funcs,
        method='SLSQP',
        tol=TOLERANCE,
    )


def get_x0_and_bounds(x, y, ψ, v, cte, eψ, a, δ):
    # TODO: impacts performance, you can do better
    x0 = np.array(
        STEPS_AHEAD * [x]
        + STEPS_AHEAD * [y]
        + STEPS_AHEAD * [ψ]
        + STEPS_AHEAD * [v]
        + STEPS_AHEAD * [cte]
        + STEPS_AHEAD * [eψ]
        + STEPS_AHEAD * [a]
        + STEPS_AHEAD * [δ]
    )

    # TODO: impacts performance, be better than this
    bounds = (
        6*STEPS_AHEAD * [(None, None)]
        + STEPS_AHEAD * [(-1, 1)]
        + STEPS_AHEAD * [(-STEER_BOUND, STEER_BOUND)]
    )

    return x0, bounds


def transform_into_cars_coordinate_system(pts, x_lat, y_lat, cos_ψ_lat, sin_ψ_lat):
    diff = (pts - [x_lat, y_lat])
    pts_car = np.zeros_like(diff)
    pts_car[:, 0] = cos_ψ_lat * diff[:, 0] + sin_ψ_lat * diff[:, 1]
    pts_car[:, 1] = sin_ψ_lat * diff[:, 0] - cos_ψ_lat * diff[:, 1]
    return pts_car


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

    steer = -0.5
    throttle = 0.5
    target_speed = args.target_speed

    depth_array = None

    cost_func, cost_grad_func, constr_funcs = get_func_constraints_and_bounds(target_speed)

    # TEMPORARY BEGIN
    # init = (1, 1, 1, 60, 1, 1.2, 1, 2, 3, 4)
    # x0, bounds = get_x0_and_bounds(1.1, 1.1, 1.1, 60.5, 1.1, 1.1, 0.5, 0.5)
    #
    # start_time = time.time()
    # for i in range(10):
    #     result = minimize_cost(cost_func, cost_grad_func, constr_funcs, bounds, x0, init)
    # print('Took {:.4f}s'.format((time.time() - start_time) / 10))
    #
    # if 'success' not in result.message:
    #     print(result)
    #     import ipdb; ipdb.set_trace()
    #
    # prev_cte = 0
    # TEMPORARY END

    with make_carla_client(args.host, args.port) as client:
        print('CarlaClient connected')
        for episode in range(0, number_of_episodes):
            # Start a new episode.
            storage = np.random.rand(IMAGE_SIZE[0], IMAGE_SIZE[1], frames_per_episode).astype(np.float16)
            log_dicts = frames_per_episode * [None]

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
            for frame in range(frames_per_episode):

                # Read the data produced by the server this frame.
                measurements, sensor_data = client.read_data()

                # # Print some of the measurements.
                # print_measurements(measurements)

                # Get current location
                x = measurements.player_measurements.transform.location.x
                y = measurements.player_measurements.transform.location.y
                location = np.array([x, y])
                # Initial: array([ -5.29999971, 250.27999878])

                orient = measurements.player_measurements.transform.orientation
                # Initial:
                # x: 0.7071127891540527
                # y: -0.7071007490158081

                v = measurements.player_measurements.forward_speed * 3.6 # km / h
                ψ = np.arctan2(orient.y, orient.x)

                # Get closest point's distance
                dists = norm(pts_2D - location, axis=1)
                which_closest = np.argmin(dists)
                indeces = which_closest + STEPS_POLY*np.arange(POLY_DEGREE+1)
                indeces = indeces % spline_points
                pts = pts_2D[indeces]

                v_lat = v + LAT * throttle
                ψ_lat = ψ - LAT * (v_lat * steer / Lf)

                cos_ψ_lat = np.cos(ψ_lat)
                sin_ψ_lat = np.sin(ψ_lat)

                x_lat = x + LAT * (v_lat * cos_ψ_lat)
                y_lat = y + LAT * (v_lat * sin_ψ_lat)

                pts_car = transform_into_cars_coordinate_system(pts, x_lat, y_lat, cos_ψ_lat, sin_ψ_lat)

                poly = np.polyfit(pts_car[:, 0], pts_car[:, 1], POLY_DEGREE)

                poly = poly[::-1]
                cte = poly[0]
                eψ = -np.arctan(poly[1])

                init = (0, 0, 0, v_lat, cte, eψ, *poly)
                x0, bounds = get_x0_and_bounds(0, 0, 0, v, cte, eψ, 0, 0)
                result = minimize_cost(cost_func, cost_grad_func, constr_funcs, bounds, x0, init)

                success = ('success' in result.message)

                if success:
                    steer = result.x[-STEPS_AHEAD]
                    throttle = result.x[-2*STEPS_AHEAD]

                # text = (
                #     'steer: {:.2f}'
                #     ' cte: {:.2f}'
                #     # ' throttle: {:.2f}'
                #     ' v: {:.2f}'
                #     .format(steer, cte, v)
                # )
                # print(text)

                # steer = -0.6 * cte - 5.5 * (cte - prev_cte)
                # prev_cte = cte
                # throttle = clip_throttle(throttle, v, target_speed)

                client.send_control(
                    steer=steer,
                    throttle=throttle,
                    brake=0.0,
                    hand_brake=False,
                    reverse=False
                )

                depth_array = np.log(sensor_data['CameraDepth'].data).astype('float16')
                storage[..., frame] = depth_array

                one_log_dict = {
                    'frame': frame,
                    'x': x,
                    'y': y,
                    'orient_x': orient.x,
                    'orient_y': orient.y,
                    'steer': steer,
                    'throttle': throttle,
                    'speed': v,
                    'psi': ψ,
                    'cte': cte,
                    'epsi': eψ,
                }
                for i in range(4):
                    one_log_dict['poly{}'.format(i)] = poly[i]

                for i in range(pts_car.shape[0]):
                    for j in range(pts_car.shape[1]):
                        one_log_dict['pts_car_{}_{}'.format(i, j)] = pts_car[i][j]

                log_dicts[frame] = one_log_dict

            np.save('depth_data{}.npy'.format(episode), storage)
            pd.DataFrame(log_dicts).to_csv('log{}.txt'.format(episode), index=False)


def print_measurements(measurements):
    player_measurements = measurements.player_measurements
    message = 'Vehicle at ({pos_x:.1f}, {pos_y:.1f}), '
    message += '{speed:.0f} km/h, '
    message = message.format(
        pos_x=player_measurements.transform.location.x,
        pos_y=player_measurements.transform.location.y,
        speed=player_measurements.forward_speed * 3.6,  # m/s -> km/h
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
