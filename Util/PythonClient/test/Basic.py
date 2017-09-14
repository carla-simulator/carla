import test


from lib.carla_util import TestCarlaClientBase
from lib.util import make_client


class UseCase(test.CarlaServerTest):
    def run(self):
        with make_client(TestCarlaClientBase, self.args) as client:
            for x in xrange(0, 5):
                client.start_episode()
                client.loop_on_agent_client(iterations=100)


class SynchronousMode(test.CarlaServerTest):
    def run(self):
        with make_client(TestCarlaClientBase, self.args) as client:
            for x in xrange(0, 5):
                client.start_episode(SynchronousMode=True)
                client.loop_on_agent_client(iterations=100)


class GetAgentsInfo(test.CarlaServerTest):
    def run(self):
        with make_client(TestCarlaClientBase, self.args) as client:
            for x in xrange(0, 3):
                client.start_episode(
                    SynchronousMode=True,
                    SendNonPlayerAgentsInfo=True,
                    NumberOfVehicles=60,
                    NumberOfPedestrians=90)
                client.loop_on_agent_client(iterations=50)
