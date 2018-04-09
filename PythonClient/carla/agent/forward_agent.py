
from carla.agent.agent import Agent
from carla.client import VehicleControl


class ForwardAgent(Agent):
    """
    Simple derivation of Agent Class,
    A trivial agent agent that goes straight
    """
    def run_step(self, measurements, sensor_data, directions, target):
        control = VehicleControl()
        control.throttle = 0.9

        return control
