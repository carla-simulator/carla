import carla

from challenge.data_provider import DataProvider

class AutonomousAgent():
    def __init__(self):
        # this dictionary will contain buffers with incoming sensor data indexed by name
        self.data_provider = DataProvider()

        self.setup()

    def setup(self):
        """
        Initialize everything needed by your agent.
        """
        pass

    def sensors_setup(self):
        """
        Define the sensor suite required by the agent

        :return: a list containing the required sensors in the following format:

        [
            ['sensor.camera.rgb', {'x':x_rel, 'y': y_rel, 'z': z_rel,
                                   'yaw': yaw, 'pitch': pitch, 'roll': roll,
                                   'width': width, 'height': height, 'fov': fov}, 'Sensor01'],
            ['sensor.camera.rgb', {'x':x_rel, 'y': y_rel, 'z': z_rel,
                                   'yaw': yaw, 'pitch': pitch, 'roll': roll,
                                   'width': width, 'height': height, 'fov': fov}, 'Sensor02'],

            ['sensor.lidar.ray_cast', {'x':x_rel, 'y': y_rel, 'z': z_rel,
                                       'yaw': yaw, 'pitch': pitch, 'roll': roll}, 'Sensor03']
        ]

        """
        sensors = []

        return sensors

    def __call__(self):
        input_data = self.data_provider.get_data()

        control = self.run_step(input_data)
        control.manual_gear_shift = False

        return control

    def run_step(self):
        """
        Execute one step of navigation.
        :return: control
        """
        pass

    def all_sensors_ready(self):
        return self.data_provider.all_sensors_ready()