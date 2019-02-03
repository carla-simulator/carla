from enum import Enum
import logging
import subprocess
import time

class Track(Enum):
    """
    Track modality.
    """
    SENSORS = 1
    NO_RENDERING = 2

class ServerManager():
    def __init__(self, opt_dict):
        log_level = logging.INFO
        logging.basicConfig(format='%(levelname)s: %(message)s', level=log_level)

        if 'CARLA_SERVER' in opt_dict:
            self._carla_server_binary = opt_dict['CARLA_SERVER']
        else:
            logging.error('CARLA_SERVER binary not provided!')

        self._proc = None
        self._outs = None
        self._errs = None

    def reset(self, map_id, track_id=Track.SENSORS, port=2000, human_flag=False):
        # first we check if there is need to clean up
        if self._proc is not None:
            logging.info('Stoppoing previous server [PID=%s]', self._proc.pid)
            self._proc.kill()
            self._outs, self._errs = self._proc.communicate()

        # temporary config file
        conf_file = '.tempconf.ini'

        exec_command = []
        exec_command += self._carla_server_binary
        exec_command += map_id

        exec_command += '-world-port={}'.format(port)
        if not human_flag:
            exec_command += '-benchmark'
            exec_command += '-fps=20'

        if track_id == Track.NO_RENDERING:
            # create ini file
            with open(conf_file, 'w') as fd:
                fd.write('[CARLA / Server]\n')
                fd.write('DisableRendering = true\n')

            exec_command += '-carla-settings={}'.format(conf_file)

        print(exec_command)
        self._proc = subprocess.Popen(exec_command)

    def wait_until_ready(self):
        ready = False
        while not ready:
            outs, errs = self._proc.communicate()

            # we check current status
            print(outs)
            ready = True

            # let's wait
            time.sleep(0.8)