# #from keras import backend as K
# #K.tensorflow_backend._get_available_gpus()

# from torchvision import datasets, transforms, models
# import torch
# from torch import nn
# from torch import optim
# import torch.nn.functional as F

# device = torch.device("cuda" if torch.cuda.is_available() else "cpu")
# model = models.resnet50(pretrained=True)
# print(model)
# print("\n\n\n")
# # Freeze parameters so we don't backprop through them
# for param in model.parameters():
#     param.requires_grad = False
    
# model.fc = nn.Sequential(nn.Linear(2048, 512),
#                                  nn.ReLU(),
#                                  nn.Dropout(0.2),
#                                  nn.Linear(512, 3),
#                                  nn.LogSoftmax(dim=1))
# criterion = nn.NLLLoss()
# optimizer = optim.Adam(model.fc.parameters(), lr=0.003)
# model.to(device)
# print(model)
# import tensorflow as tf
# print(tf.keras.Input(shape=(240,360,3)))


# import torch
# device = torch.device("cuda:1" if torch.cuda.is_available() else "cpu")
# print(device)
# print(torch.cuda.get_device_name(0))

import gym

from gym import envs
all_envs = envs.registry.all()
env_ids = [env_spec.id for env_spec in all_envs]
print(sorted(env_ids))

env = gym.make('Breakout-v0', render_mode='human')
done = False
env.reset()
print("Obs :", env.observation_space)
print("ASpace: ", env.action_space)
while not done:
    state, reward, done, _ = env.step(env.action_space.sample())
   # print("State shape ", state.shape)
   # print("reward, ", reward)
    env.render()
env.close()
