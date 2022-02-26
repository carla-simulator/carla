import random
import numpy as np
from numba import jit, cuda

from collections import deque

import tensorflow as tf
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

# from tensorflow import keras
# from tensorflow.keras.optimizers import Adam

class DQNAgent:
    def __init__(self, state_size, action_size):
        self.state_size = state_size # ()
        self.action_size = action_size
        self.memory = deque(maxlen=100000)
        self.gamma = 0.9    # discount rate
        self.loss_list = []
        self.epsilon = 1  # exploration rate
        self.epsilon_min = 0.01
        self.epsilon_decay = 0.99995 # changed from 0.995, maybe it can be even slower 
        self.learning_rate = 0.001
        self.model = self._build_model()

    # @jit
    def _build_model(self):
        # OUR MODEL WILL BE RESNET WITH SOME LINEAR LAYERS AT THE END
        # INPUT = (1280, 720, 3) STACK OF 3 UNCERTAINTY MAPS
        # OUTPUT = (1, 13) 
        # ResNet 50
        base_model = tf.keras.applications.ResNet50(weights='imagenet', include_top=False, input_shape=(480, 640, 3))
        base_model.trainable = False
        # Additional Linear Layers
        inputs = tf.keras.Input(shape=(480, 640, 3))
        #print(inputs)
        x = base_model(inputs, training=False)
        x = tf.keras.layers.GlobalAveragePooling2D()(x)
        # x = keras.layers.Dropout(0.2)(x)
        x = tf.keras.layers.Flatten()(x)
        x = tf.keras.layers.Dense(units=40, activation='relu')(x)
        # x = keras.layers.Dropout(0.2)(x)
        output = tf.keras.layers.Dense(units=13, activation='linear')(x)
        # Compile the Model
        model = tf.keras.Model(inputs, output)
        model.compile(loss='mse', optimizer=tf.keras.optimizers.Adam(learning_rate=self.learning_rate))
        return model


    def memorize(self, state, action, reward, next_state, done, frameNumber):
        self.memory.append((state, action, reward, next_state, done, frameNumber))

    def load(self, name):
        self.model.load_weights(name)

    def save(self, name):
        self.model.save_weights(name)
        
    def act(self, state):
        # randomly select action
        if np.random.rand() <= self.epsilon:
            return random.randrange(self.action_size)
            #return random.randint(5,7)
        # use NN to predict action
        state = np.expand_dims(state, axis=0)
        act_values = self.model.predict(state)
        return np.argmax(act_values[0])  
    
    
    def replay(self, batch_size):
        minibatch = random.sample(self.memory, batch_size)
        states, targets_f = [], []
        for state, action, reward, next_state, done, frameNumber in minibatch:
            # if done, set target = reward
            target = reward
            # if not done, predict future discounted reward with the Bellman equation
            next_state = np.expand_dims(next_state, axis=0)
            if not done:
                values = self.model.predict(next_state)
                target = (reward + self.gamma * np.argmax(values[0]))

            state = np.expand_dims(state, axis=0)       
            target_f = self.model.predict(state)
            target_f[0][action] = target 

            # filtering out states and targets for training
            states.append(state[0])
            targets_f.append(target_f[0])

        # RUN ONE ITERATION OF GRADIENT DESCENT
        history = self.model.fit(np.array(states), np.array(targets_f), epochs=1, verbose=0)
        # Keeping track of loss
        loss = history.history['loss'][0]
        if self.epsilon > self.epsilon_min:
            self.epsilon *= self.epsilon_decay
        return loss