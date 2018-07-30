


from carla.agent.agent import Agent


class CommandFollower(Agent):
    """
    Simple derivation of Agent Class,
    A command follower that follows high level commands and goes driving around the city
    Not yet implemented
    """

    def run_step(self, measurements, sensor_data, directions, target):

        raise NotImplementedError("Command follower not yet implemented")
