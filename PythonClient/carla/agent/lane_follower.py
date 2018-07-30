


from carla.agent.agent import Agent


class LaneFollower(Agent):
    """
    Simple derivation of Agent Class,
    A lane follower that randomly goes driving around the city
    Not yet implemented
    """

    def run_step(self, measurements, sensor_data, directions, target):

        raise NotImplementedError("Lane follower not yet implemented")
