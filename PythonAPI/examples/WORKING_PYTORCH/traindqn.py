from dqnAgent import Agent
from collections import deque
import matplotlib.pyplot as plt
import numpy as np
import torch 
from carla_agent_pytorch import CarEnv
import torchvision
import gym

env = gym.make('Breakout-v4', render_mode='human')
agent = Agent(state_size=(3,210,160),action_size=4,seed=0)
#env = CarEnv()
#plot the scores

fig = plt.figure()
ax = fig.add_subplot(111)

def dqn(n_episodes= 20_000, max_timesteps = 1000, epsilon_start=1.0, epsilon_min = 0.01,
       epsilon_decay=0.99996):
    """Deep Q-Learning
    
    Params
    ======
        n_episodes (int): maximum number of training epsiodes
        max_timesteps (int): maximum number of timesteps per episode
        epsilon_start (float): starting value of epsilon, for epsilon-greedy action selection
        epsilon_min (float): minimum value of epsilon 
        epsilon_decay (float): mutiplicative factor (per episode) for decreasing epsilon
        
    """
    scores = [] # list containing score from each episode
    epsilon = epsilon_start
    env.render()

    for i_episode in range(1, n_episodes+1):
        #state = env.restart()
        state = env.reset()
        score = 0
        done = False
        print("\n\nEpisode Number: ",i_episode)
        for t in range(max_timesteps):
            action = agent.act(state,epsilon)
            next_state,reward,done,_ = env.step(action)
            agent.step(state,action,reward,next_state,done)
            ## above step decides whether we will train(learn) the network
            ## actor (local_qnetwork) or we will fill the replay buffer
            ## if len replay buffer is equal to the batch size then we will
            ## train the network or otherwise we will add experience tuple in our
            ## replay buffer.
            state = next_state
            score += reward
            if done:
                break

            epsilon = max(epsilon*epsilon_decay,epsilon_min)## decrease the epsilon

        plt.plot(np.arange(len(scores)),scores)
        plt.ylabel('Score')
        plt.xlabel('Episode #')
        plt.savefig('breakout.png')

        #Print reward every episode
        print("Episode Rewards:  ", score)
        scores.append(score)

        #Save model every 100 episodes
        if i_episode %100==0:
            #print('\rEpisode {}\tAverage Score {:.2f}'.format(i_episode,np.mean(scores_window)))
            torch.save(agent.qnetwork_local.state_dict(),'checkpoint.pth')

        # End of episode - destroy agents
        # for actor in env.actor_list:
        #     actor.destroy()        
    return scores

scores= dqn()