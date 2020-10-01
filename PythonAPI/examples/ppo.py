import gym
from gym import spaces
from stable_baselines3 import PPO
from stable_baselines3.common.cmd_util import make_vec_env
from stable_baselines3.common.vec_env import VecFrameStack, SubprocVecEnv, VecNormalize, DummyVecEnv, VecEnv, VecTransposeImage
from stable_baselines3.common.env_checker import check_env
from stable_baselines3.common.callbacks import EvalCallback
from stable_baselines3.common.monitor import Monitor
from vec_resize import VecResizeImage
from carla_wrappers import CarlaWrapper
import numpy as np
import logging
import argparse
import cv2
import carla
from rl_control import World, HUD
import matplotlib.pyplot as plt
import torch.nn.functional as F
import torch
from torch.autograd import Variable

try:
    import pygame
except ImportError:
    raise RuntimeError('cannot import pygame, make sure pygame package is installed')

def resize_env_obs(obs):
    '''
    function used to downsample the original observation from carla env to (90, 160)
    '''
    tmp = F.interpolate(obs, (90, 160), align_corners=True, mode='bilinear')
    tmp = tmp.cpu().numpy()
    return tmp

class CarlaEnv(gym.Env):
    """Custom Environment that follows gym interface"""
    metadata = {'render.modes': ['human']}

    def __init__(self, args):
        super(CarlaEnv, self).__init__()
        # Define action and observation space
        # They must be gym.spaces objects
        # Example when using discrete actions:
        self.resize = args.resize
        self.action_space = spaces.Discrete(4)
        # Example for using image as input:
        if self.resize:
            self.observation_space = spaces.Box(low=0, high=255,
                                            shape=(90, 160, 3), dtype=np.uint8)
        self.observation_space = spaces.Box(low=0, high=255,
                                            shape=(args.height, args.width, 3), dtype=np.uint8)
        # TODO: action and obs space
        
        pygame.init()
        pygame.font.init()
        print(args)
        self.world = None
        
        try:
            self.client = carla.Client(args.host, args.port)
            self.client.set_timeout(100.0)  
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
        # if self.world.steps % 900 == 0:
        #     cv2.imwrite(("data/source_domain/images/carla_%d_%d.jpg" % (episode, world.steps)), obs.sensor_data.rgb_img)
        #     cv2.imwrite(("data/source_domain/depth/carla_%d_%d.jpg" % (episode, world.steps)), obs.sensor_data.depth_log_img)
        #     cv2.imwrite(("data/source_domain/segmentation/carla_%d_%d.jpg" % (episode, world.steps)), obs.sensor_data.seg_csp_img)
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
    argparser.add_argument(
        '--resize',
        action='store_true',
        help='True if the observation form the environment needs to be resized to 90 * 160'
    )
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

def callable_env(args):
    def _init():
        env = CarlaEnv(args=args) # TODO
        env = Monitor(env, "./log/")
        # env = CarlaWrapper(env)
        return env
    return _init


def make_env(args):
    # env = CarlaEnv(args=args) # TODO
    # # env = Monitor(env, "./log/")

    # check_env(env)
    # print("Finish check env")
    # import pdb
    # pdb.set_trace()
    env = DummyVecEnv([callable_env(args)])
    # env = make_vec_env(lambda: env, n_envs=1)
    # env = VecResizeImage(env)
    env = VecFrameStack(env, 4)
    env = VecTransposeImage(env)
    return env

def test_visualize(env):
    '''
    function used to visualize the output observation of the environment
    '''
    for i in range(0,4):
        obs, reward, done, info = env.step([0])
    for i in range (0, 4):
       
        print(obs.shape)
        tmp = obs[0][i*3:i*3+3, :, :]
        tmp = np.transpose(tmp, (1, 2, 0))
        print(tmp.shape)
        plt.imshow(tmp)
        plt.show()


    

def game_loop(args):
    env = None
    try:
        env = make_env(args)
        # test_env = make_env(args)
        # eval_callback = EvalCallback(eval_env, eval_freq=500,
            # deterministic=True, render=False)
        model = PPO('CnnPolicy', env, verbose=1, batch_size=8, n_steps=8, device="auto")
        # model.learn(total_timesteps=10000)

        obs = env.reset()
        print(obs.shape)
        test_visualize(env)
        for episode in range(args.num_episodes):
            print("Starting episode %d" % episode)
            done = False
            while not done:
                action, _states = model.predict(obs, deterministic=True)
                action = [0]
                print(action)
                obs, reward, done, info = env.step(action)
                print(info)
                if not done:
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