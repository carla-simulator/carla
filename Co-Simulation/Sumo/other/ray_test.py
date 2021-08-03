import ray
import time
from ray_class import A, B, C, D

ray.init()

# a1 = A.remote()
# a2 = A.remote()
# t = time.time()
# a1_pid = a1.sleep.remote(5)
# a2_pid = a2.sleep.remote(10)
# print(ray.get([a1_pid, a2_pid]))
# print(time.time() - t)


# b1 = B()
# b2 = B()
# t = time.time()
# b1_pid = b1.sleep(5)
# b2_pid = b2.sleep(10)
# print(ray.get([b1_pid, b2_pid]))
# # a_pid = a.sleep(5)
# # b_pid = b.sleep(10)
# print(time.time() - t)


# c1 = C()
# c2 = C()
# t = time.time()
# c1_pid = c1.sleep(5)
# c2_pid = c2.sleep(10)
# print(ray.get([c1_pid, c2_pid]))
# # a_pid = a.sleep(5)
# # b_pid = b.sleep(10)
# print(time.time() - t)


d1 = D()
d2 = D()
t = time.time()
d1_pid = d1.sleep(5)
d2_pid = d2.sleep(10)
print(ray.get([d1_pid, d2_pid]))
# a_pid = a.sleep(5)
# b_pid = b.sleep(10)
print(time.time() - t)
d1.inc()
d2.inc()
