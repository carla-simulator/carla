import glob
import os
import sys
import random
import time
import numpy as np
import cv2
import math
import pickle
from collections import deque
import tensorflow as tf
import matplotlib.pyplot as plt

gpus = tf.config.experimental.list_physical_devices('GPU')
if gpus:
    try:
        # Currently, memory growth needs to be the same across GPUs
        for gpu in gpus:
            tf.config.experimental.set_memory_growth(gpu, True)
        logical_gpus = tf.config.experimental.list_logical_devices('GPU')
        print(len(gpus), "Physical GPUs,", len(logical_gpus), "Logical GPUs")
    except RuntimeError as e:
        # Memory growth must be set before GPUs have been initialized
        print(e)

from threading import Thread

from tqdm import tqdm

try:
    sys.path.append(glob.glob('../carla/dist/carla-*%d.%d-%s.egg' % (
        sys.version_info.major,
        sys.version_info.minor,
        'win-amd64' if os.name == 'nt' else 'linux-x86_64'))[0])
except IndexError:
    pass
import carla
from carla import ColorConverter as cc


SHOW_PREVIEW = True
IM_WIDTH = 360
IM_HEIGHT = 240
SECONDS_PER_EPISODE = 30
REPLAY_MEMORY_SIZE = 20_000
MIN_REPLAY_MEMORY_SIZE = 1_000
MINIBATCH_SIZE = 16
PREDICTION_BATCH_SIZE = 1
TRAINING_BATCH_SIZE = MINIBATCH_SIZE // 4
UPDATE_TARGET_EVERY = 5
MODEL_NAME = "Xception"

MEMORY_FRACTION = 0.4
MIN_REWARD = -200

EPISODES = 40_000

DISCOUNT = 0.99
epsilon = 1
EPSILON_DECAY = 0.99995 #0.95 #0.99975
MIN_EPSILON = 0.001

AGGREGATE_STATS_EVERY = 10
PREDICTED_ANGLES = [-1, 0, 1]

class CarEnv:
    SHOW_CAM = SHOW_PREVIEW
    im_width = IM_WIDTH
    im_height = IM_HEIGHT
    front_camera = None

    def __init__(self):
        self.client = carla.Client("127.0.0.1", 2000)
        self.client.set_timeout(10.0)
        self.world = self.client.get_world()
        self.world.constant_velocity_enabled=True
        self.blueprint_library = self.world.get_blueprint_library()
        self.model_3 = self.blueprint_library.filter("model3")[0]


    def restart(self):
        self.collision_hist = deque(maxlen=2000)
        self.actor_list = deque(maxlen=2000)
        self.lane_invasion = deque(maxlen=2000)
        self.obstacle_distance = deque(maxlen=2000)

        self.transform = random.choice(self.world.get_map().get_spawn_points())
        self.vehicle = self.world.spawn_actor(self.model_3, self.transform)
        self.actor_list.append(self.vehicle)

        # self.rgb_cam = self.blueprint_library.find('sensor.camera.rgb')
        # self.rgb_cam.set_attribute("image_size_x", f"{self.im_width}")
        # self.rgb_cam.set_attribute("image_size_y", f"{self.im_height}")
        # self.rgb_cam.set_attribute("fov", f"110")

        self.ss_cam = self.blueprint_library.find('sensor.camera.semantic_segmentation')
        self.ss_cam.set_attribute("image_size_x", f"{self.im_width}")
        self.ss_cam.set_attribute("image_size_y", f"{self.im_height}")
        self.ss_cam.set_attribute("fov", f"110")

        transform = carla.Transform(carla.Location(x=2.5, z=0.7))
        self.sensor = self.world.spawn_actor(self.ss_cam, transform, attach_to=self.vehicle)
        self.actor_list.append(self.sensor)
        self.sensor.listen(lambda data: self.process_img(data))

        self.vehicle.apply_control(carla.VehicleControl(throttle=0.0, brake=0.0))
        self.vehicle.enable_constant_velocity(carla.Vector3D(11, 0, 0))
        time.sleep(4)

        col_sensor = self.blueprint_library.find("sensor.other.collision")
        self.col_sensor = self.world.spawn_actor(col_sensor, transform, attach_to=self.vehicle)
        self.actor_list.append(self.col_sensor)
        self.col_sensor.listen(lambda event: self.collision_data(event))

        lane_sensor = self.blueprint_library.find("sensor.other.lane_invasion")
        self.lane_sensor = self.world.spawn_actor(lane_sensor, transform, attach_to=self.vehicle)
        self.actor_list.append(self.lane_sensor)
        self.lane_sensor.listen(lambda lanesen: self.lane_data(lanesen))
        
        obs_sensor = self.blueprint_library.find("sensor.other.radar")
        self.obs_sensor = self.world.spawn_actor(
            obs_sensor,
            carla.Transform(
                carla.Location(x=2.8, z=1.0),
                carla.Rotation(pitch=5)),
            attach_to=self.vehicle)
        self.actor_list.append(self.obs_sensor)
        self.obs_sensor.listen(lambda obsen: self.obs_data(obsen))
        
        while self.front_camera is None:
            time.sleep(0.01)

        self.episode_start = time.time()
        self.vehicle.apply_control(carla.VehicleControl(throttle=0.0, brake=0.0))

        return self.front_camera

    def collision_data(self, event):
        self.collision_hist.append(event)

    def lane_data(self, lanesen):
        lane_types = set(x.type for x in lanesen.crossed_lane_markings)
        text = ['%r' % str(x).split()[-1] for x in lane_types]
        self.lane_invasion.append(text[0])

    def obs_data(self, obsen):
        for detect in obsen:
            if detect.depth <= float(30.0):
                self.obstacle_distance.append(detect.depth)
    
    def process_img(self, image):
        image.convert(cc.CityScapesPalette)
        i = np.array(image.raw_data, dtype='uint8')
        i2 = i.reshape((self.im_height, self.im_width, 4))
        i3 = i2[:, :, :3]       
        if self.SHOW_CAM:
            cv2.imshow("test", i3)
            cv2.waitKey(1)
        self.front_camera = i3

    def get_rewards(self,angle):
        done = False

        # Assign Reward for Collision
        reward_collision = 0
        if len(self.collision_hist) > 0:
            reward_collision = -6
            done = True

        #Crossing lanes
        reward_lane = 0
        if len(self.lane_invasion) > 0:
            lane_invasion_error = self.lane_invasion.pop()
            #print(lane_invasion_error)
            if 'Broken' in lane_invasion_error:
                reward_lane += -2
            elif 'Solid' in lane_invasion_error:
                reward_lane += -4
        
        #Assign reward for obstacle distance
        reward_obs = 0
        # distance = 40
        # if len(self.obstacle_distance) > 0 :
        #     distance = self.obstacle_distance.pop()
        #     reward_obs = int(-40 + distance) if (distance < 10) else int(distance)
            
        # Assign reward for steering angle
        # reward_steer = 0
        # if abs(angle) > 0.2:
        #     reward_steer = -0.6

        total_reward = reward_collision + reward_lane
        return total_reward, done


    def step(self, actionIndex):
        angle = PREDICTED_ANGLES[actionIndex]
        self.vehicle.apply_control(carla.VehicleControl(steer=angle))
        v = self.vehicle.get_velocity()
        #self.vehicle.set_target_velocity(v)

        kmh = int(3.6 * math.sqrt(v.x**2 + v.y**2 + v.z**2))
        speed_reward = 1

        total_rewards, done = self.get_rewards(angle)
        total_rewards+=speed_reward

        if self.episode_start + SECONDS_PER_EPISODE < time.time():
            done = True

        return self.front_camera, total_rewards, done, None


class DQNAgent:
    def __init__(self, state_size, action_size):
        self.state_size = state_size # ()
        self.action_size = action_size
        self.memory = deque(maxlen=REPLAY_MEMORY_SIZE)
        self.training_initialized = False
        self.gamma = 0.9    # discount rate
        self.loss_list = deque(maxlen=20000)
        self.epsilon = 1  # exploration rate
        self.epsilon_min = MIN_EPSILON #0.01
        self.epsilon_decay = EPSILON_DECAY #0.99995 # changed to 0.95
        self.learning_rate = 0.001
        self.model = self._build_model()
        self.target_model = self._build_model()
        self.episode_number = 0
        self.terminate = False

    # @jit
    def _build_model(self):
        base_model = tf.keras.applications.ResNet50(weights='imagenet', include_top=False, input_shape=(240, 360, 3))
        base_model.trainable = False
        # Additional Linear Layers
        inputs = tf.keras.Input(shape=(240, 360, 3))
        x = base_model(inputs, training=False)
        x = tf.keras.layers.GlobalAveragePooling2D()(x)
        x = tf.keras.layers.Flatten()(x)
        x = tf.keras.layers.Dense(units=40, activation='relu')(x)
        output = tf.keras.layers.Dense(units=3, activation='linear')(x)
        # Compile the Model
        model = tf.keras.Model(inputs, output)
        model.compile(loss='mse', optimizer=tf.keras.optimizers.Adam(learning_rate=self.learning_rate))
        print(model.summary)
        return model


    def memorize(self, state, action, reward, next_state, done):
        transition = (state, action, reward, next_state, done)
        self.memory.append(transition)
        # with open('_out/memory_list.txt', "wb") as myfile:
        #     myfile.write(str(transition).rstrip('\n') +" ")


    def load(self, name):
        self.model.load_weights(name)

    def save(self, name):
        self.model.save_weights(name)
        
    def act(self, state):
        # randomly select action
        if np.random.rand() <= self.epsilon:
            return (random.randrange(self.action_size),True)
        # use NN to predict action
        state = np.expand_dims(state, axis=0)
        act_values = self.model.predict(state)
        return (np.argmax(act_values[0]), False)
    
    
    def replay(self):
        with tf.device('/gpu:0'):
            if len(self.memory) < MIN_REPLAY_MEMORY_SIZE:
                return

            minibatch = random.sample(self.memory, MINIBATCH_SIZE)
            current_states = np.array([transition[0] for transition in minibatch])/255

            current_qs_list = self.model.predict(current_states, PREDICTION_BATCH_SIZE)
            new_current_states = np.array([transition[3] for transition in minibatch])/255

            future_qs_list = self.target_model.predict(new_current_states, PREDICTION_BATCH_SIZE)

            X = []
            y = []

            for index, (current_state, action, reward, new_state, done) in enumerate(minibatch):
                if not done:
                    max_future_q = np.max(future_qs_list[index])
                    new_q = reward + DISCOUNT * max_future_q
                else:
                    new_q = reward

                current_qs = current_qs_list[index]
                current_qs[action] = new_q

                X.append(current_state)
                y.append(current_qs)

            history = self.model.fit(np.array(X)/255, np.array(y), batch_size=TRAINING_BATCH_SIZE, verbose=0, shuffle=False)  
            loss = history.history['loss'][0]

            self.loss_list.append(loss)

            if self.episode_number%5 == 0 and self.episode_number > 1: 
                self.target_model.set_weights(self.model.get_weights())

    def get_qs(self, state):
        return self.model.predict(np.expand_dims(state, axis=0))[0]

    def train_in_loop(self):
        self.training_initialized = True

        while True:
            if self.terminate:
                return
            self.replay()
            time.sleep(0.01)

    def load_memory(self):
        try:
            print("Prev length: ", len(self.memory))
            with open('memory1.dump', 'rb') as f:
                agent.memory = pickle.load(f)
            print("New length: ", len(self.memory))
        except:
            print("New length: ", len(self.memory))
            return


if __name__ == '__main__':
    FPS = 60
    # For stats
    ep_rewards = [-200]

    # For more repetitive results
    random.seed(1)
    np.random.seed(1)
    tf.random.set_seed(1)

    # Memory fraction, used mostly when training multiple agents
    #gpu_options = tf.compat.v1.GPUOptions(per_process_gpu_memory_fraction=MEMORY_FRACTION)
    # backend.set_session(tf.compat.v1.Session(config=tf.compat.v1.ConfigProto(gpu_options=gpu_options)))

    # Create models folder
    if not os.path.isdir('models'):
        os.makedirs('models')

    # Create agent and environment
    
    agent = DQNAgent(state_size=(240,360,3),action_size=3)
    #agent.model.load_weights('_out/savedWeights.h5')
    #agent.model = tf.keras.models.load_model('_out/savedModel_12500.h5')
    #agent.target_model = tf.keras.models.load_model('_out/savedTargetModel_12500.h5')
    
    # load Agent Memory 
    #agent.load_memory()
    
    env = CarEnv()

    #load these
    episode_loss = []
    episode_number = []

    # with open("_out/loss_lists.txt", "r") as text_file:
    #     content = text_file.read()
    #     file_content = content.splitlines()

    #     episode_number = [int(i) for i in file_content[0].split()]
    #     episode_loss = [float(i) for i in file_content[3].split()]

    #load these
    reward_list = []
    ten_reward_array = []
    episode_list = []

    # with open("_out/reward_lists.txt", "r") as text_file:
    #     content = text_file.read()
    #     file_content = content.splitlines()

    #     episode_list = [int(i) for i in file_content[0].split()]
    #     ten_reward_array = [float(i) for i in file_content[3].split()]

    elapsed_time = []
    total_steps = []
    save_before = False

    # Start training thread and wait for training to be initialized
    trainer_thread = Thread(target=agent.train_in_loop, daemon=True)
    trainer_thread.start()
    while not agent.training_initialized:
        time.sleep(0.01)

    # Initialize predictions - forst prediction takes longer as of initialization that has to be done
    # It's better to do a first prediction then before we start iterating over episode steps
    #agent.get_qs(np.ones((env.im_height, env.im_width, 3)))
    
    # Iterate over episodes
    #current_state = env.start()
    with tf.device('/gpu:1'):
        for episode in tqdm(range(0, EPISODES + 1), ascii=True, unit='episodes'):
            agent.episode_number = episode
            print("eps: ", agent.epsilon)
            start_time = time.time()

            env.collision_hist = deque(maxlen=1000)
            env.lane_invasion = deque(maxlen=1000)
            env.col_sensor = deque(maxlen=1000)

            # Restarting episode - reset episode reward and step number
            if episode%50 == 0:
                agent.model.save_weights("_out/savedWeights.h5")
            if episode % 250 == 0:
                agent.model.save(str("_out/savedModel_"+str(episode) + ".h5"))
                #agent.target_model.save(str("_out/savedTargetModel_"+str(episode) + ".h5"))
                
                if save_before:
                    num = 2
                    save_before = False
                else:
                    num = 1
                    save_before = True
                # with open(str('memory'+str(num)+'.dump'), 'wb') as f:
                #     pickle.dump(agent.memory, f)

            episode_reward = 0
            step = 1

            #Store into episode loss graph
            if episode%10 == 1 and episode > 1:
                if len(agent.loss_list) > 0:
                    episode_loss.append(sum(agent.loss_list) / len(agent.loss_list))
                    episode_number.append(episode-1)
                    agent.loss_list.clear()

                    plt.figure(1)
                    plt.ylabel('Loss')
                    plt.xlabel('Episodes')
                    plt.title('Figure 1: Average Loss over Episodes')

                    plt.plot(episode_number,episode_loss)
                    plt.savefig('_out/loss_graph.png')

                    with open('_out/loss_lists.txt', "w") as myfile:
                        for eps in episode_number:
                            myfile.write(str(eps) +" ")

                        myfile.write("\n\n\n")

                        for loss in episode_loss:
                            myfile.write(str(loss) +" ")

                        myfile.write("\n\n\n")

                ten_reward_average = sum(reward_list)/len(reward_list)
                ten_reward_array.append(ten_reward_average)
                reward_list.clear()

                plt.figure(2)
                episode_list.append(episode-1)
                plt.xlabel('Episodes')
                plt.ylabel('Rewards')
                plt.title('Figure 2: Average Rewards over Episodes')  
                plt.plot(episode_list, ten_reward_array)
                plt.savefig('_out/reward_graph.png')

                with open('_out/reward_lists.txt', "w") as myfile:
                    for eps in episode_list:
                        myfile.write(str(eps) +" ")

                    myfile.write("\n\n\n")

                    for reward in ten_reward_array:
                        myfile.write(str(reward) +" ")
                        
                    myfile.write("\n\n\n")

            
            # Reset environment and get initial episode state
            current_state = env.restart() #return front camera without actor destroy

            # Reset flag and start iterating until episode ends
            done = False
            episode_start = time.time()

            # Play for given number of seconds only
            while True:

                # This part stays mostly the same, the change is to query a model for Q values
                if np.random.random() > agent.epsilon:
                    # Get action from Q table
                    action = np.argmax(agent.get_qs(current_state))
                else:
                    # Get random action
                    action = random.randrange(3)
                    # This takes no time, so we add a delay matching 60 FPS (prediction above takes longer)
                    time.sleep(1/FPS)
                # action, busy = agent.act(current_state)

                new_state, reward, done, _ = env.step(action)

                # Transform new continous state to new discrete state and count reward
                episode_reward += reward
                
                # Every step we update replay memory
                agent.memorize(current_state, action, reward, new_state, done)

                current_state = new_state
                step += 1

                if done:
                    episode_reward /= (step-1)
                    reward_list.append(episode_reward)
                    end_time = time.time() - start_time
                    print("\nElapsed Time: ", end_time)
                    print("\nNumber of steps: ", step)
                    elapsed_time.append(end_time)
                    total_steps.append(step)
                    break

            # End of episode - destroy agents
            for actor in env.actor_list:
                actor.destroy()

            # Append episode reward to a list and log stats (every given number of episodes)
            ep_rewards.append(episode_reward)
            if not episode % AGGREGATE_STATS_EVERY or episode == 1:
                average_reward = sum(ep_rewards[-AGGREGATE_STATS_EVERY:])/len(ep_rewards[-AGGREGATE_STATS_EVERY:])
                min_reward = min(ep_rewards[-AGGREGATE_STATS_EVERY:])
                max_reward = max(ep_rewards[-AGGREGATE_STATS_EVERY:])

            # Decay epsilon
            if agent.epsilon > agent.epsilon_min:
                agent.epsilon *= agent.epsilon_decay

    print("Average Time for 360 x 240: ", sum(elapsed_time)/len(elapsed_time))
    print("Average steps for 360 x 240 ", sum(total_steps)/len(total_steps))
    # Set termination flag for training thread and wait for it to finish
    agent.terminate = True
    trainer_thread.join()
    agent.model.save('final_model.h5')