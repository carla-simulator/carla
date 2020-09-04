
import gym
from gym import spaces
from stable_baselines3 import PPO
import numpy as np
import logging
import argparse
import cv2
import carla
from rl_control import World, HUD
try:
    import pygame
except ImportError:
    raise RuntimeError('cannot import pygame, make sure pygame package is installed')

class CarlaEnv(gym.Env):
    """Custom Environment that follows gym interface"""
    metadata = {'render.modes': ['human']}

    def __init__(self, args):
        super(CarlaEnv, self).__init__()
        # Define action and observation space
        # They must be gym.spaces objects
        # Example when using discrete actions:
        self.action_space = spaces.Discrete(4)
        # Example for using image as input:
        self.observation_space = spaces.Box(low=0, high=255,
                                            shape=(720, 1280, 3), dtype=np.uint8)
        # TODO: action and obs space
        
        pygame.init()
        pygame.font.init()
        print(args)
        self.world = None
        try:
            self.client = carla.Client(args.host, args.port)
            self.client.set_timeout(2.0)  
            self.display = pygame.display.set_mode(
                (args.width, args.height),
                pygame.HWSURFACE | pygame.DOUBLEBUF)

            self.hud = HUD(args.width, args.height)
            self.clock = pygame.time.Clock()
            self.world = World(carla_world=self.client.get_world(), hud=self.hud, clock=self.clock, display=self.display, args=args)
        except Exception as e:
            print(e)
            return

    def step(self, action):
        self.clock.tick_busy_loop(60)
        raw_obs, reward, done, info = self.world.step(action)
        observation = raw_obs.sensor_data.rgb_img
        return observation, reward, done, info

    def reset(self):
        raw_obs = self.world.reset() 
        observation = raw_obs.sensor_data.rgb_img
        return observation  # reward, done, info can't be included

    def render(self, mode='human'):
        if self.world.steps % 900 == 0:
            cv2.imwrite(("data/source_domain/images/carla_%d_%d.jpg" % (episode, world.steps)), obs.sensor_data.rgb_img)
            cv2.imwrite(("data/source_domain/depth/carla_%d_%d.jpg" % (episode, world.steps)), obs.sensor_data.depth_log_img)
            cv2.imwrite(("data/source_domain/segmentation/carla_%d_%d.jpg" % (episode, world.steps)), obs.sensor_data.seg_csp_img)
        self.world.tick(self.clock)
        self.world.render(self.display)
        pygame.display.flip()

    def close (self):
        if (self.world and self.world.recording_enabled):
            self.client.stop_recorder()
        if self.world is not None:
            self.world.destroy()
        pygame.quit()


def main():
    argparser = argparse.ArgumentParser(
        description='CARLA Manual Control Client')
    argparser.add_argument(
        '-v', '--verbose',
        action='store_true',
        dest='debug',
        help='print debug information')
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
        '-a', '--autopilot',
        action='store_true',
        help='enable autopilot')
    argparser.add_argument(
        '--res',
        metavar='WIDTHxHEIGHT',
        default='1280x720',
        help='window resolution (default: 1280x720)')
    argparser.add_argument(
        '--filter',
        metavar='PATTERN',
        default='vehicle.*',
        help='actor filter (default: "vehicle.*")')
    argparser.add_argument(
        '--rolename',
        metavar='NAME',
        default='hero',
        help='actor role name (default: "hero")')
    argparser.add_argument(
        '--gamma',
        default=2.2,
        type=float,
        help='Gamma correction of the camera (default: 2.2)')
    # NOTE: number of episodes
    argparser.add_argument(
        '--num-episodes',
        default=10000,
        type=int,
        help='number of episodes for training')
    args = argparser.parse_args()

    args.width, args.height = [int(x) for x in args.res.split('x')]

    log_level = logging.DEBUG if args.debug else logging.INFO
    logging.basicConfig(format='%(levelname)s: %(message)s', level=log_level)

    logging.info('listening to server %s:%s', args.host, args.port)

    print(__doc__)

    try:

        game_loop(args)

    except KeyboardInterrupt:
        print('\nCancelled by user. Bye!')


def game_loop(args):
    env = None
    try:
        env = CarlaEnv(args=args) # TODO
        print(env)
        model = PPO('CnnPolicy', env, verbose=1)
        model.learn(total_timesteps=10000)
        obs = env.reset()

        for episode in range(args.num_episodes):
            print("Starting episode %d" % episode)
            done = False
            while not done: 
                action, _states = model.predict(obs, deterministic=True)
                obs, reward, done, info = env.step(action)
                env.render()
            obs = env.reset()
    except KeyboardInterrupt:
        print('\nCancelled by user. Bye!')
        return
    except Exception as e:
        print(e)
        return
    finally:
        if env is not None:
            env.close()


if __name__ == '__main__':

    main()