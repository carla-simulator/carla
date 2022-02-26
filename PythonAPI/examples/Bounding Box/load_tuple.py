import pickle
from collections import deque

memory = deque(maxlen=20000)
print(len(memory))
memory.append((1,2,3,4))
memory.append((5,6,7,8))
print(len(memory))
print(memory)

with open('trial.dump', 'wb') as f:
    pickle.dump(memory,f)

# memory_copy = deque(maxlen=20000)
# print(len(memory_copy))
with open('trial.dump', 'rb') as f:
    memory = pickle.load(f)
print(len(memory))
print(memory)