import ray
import time

def mysleep(t):
    time.sleep(t)


@ray.remote
class A:
    def __init__(self):
        self.inc = 0

    def sleep(self, t):
        time.sleep(t)
        self.update_inc(t)

    def update_inc(self, t):
        self.inc = self.inc + t

    def inc_p(self):
        return self.inc

    def sleep_t(self, t):
        self.sleep(t)
        return t



class B:
    def __init__(self):
        self.a = A.remote()

    def sleep(self, t):
        return self.a.sleep.remote(t)


class C:

    def __init__(self):
        self.a = A.remote()
        self.a1 = A.remote()

    def sleep(self, t):
        a_pid = self.a.sleep.remote(t)
        ray.get(a_pid)
        return self.a.sleep.remote(t)

class D(B):
    def sleep(self, t):
        a_pid = self.a.sleep_t.remote(t)
        return self.a.sleep.remote(a_pid)


    def inc(self):
        print(ray.get(self.a.inc_p.remote()))
