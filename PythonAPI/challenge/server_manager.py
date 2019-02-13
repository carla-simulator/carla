from enum import Enum
import logging
import random
import string
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

        self._proc = None
        self._outs = None
        self._errs = None

    def reset(self, map_id, track_id=Track.SENSORS, port=2000, human_flag=False):
        raise NotImplementedError("This function is to be implemented")


    def wait_until_ready(self):
        ready = False
        while not ready:
            line = str(self._proc.stdout.readline())
            if 'Bringing World' in line:
                ready = True
        time.sleep(1.0)


class ServerManagerBinary(ServerManager):
    def __init__(self, opt_dict):
        super(ServerManagerBinary, self).__init__(opt_dict)

        if 'CARLA_SERVER' in opt_dict:
            self._carla_server_binary = opt_dict['CARLA_SERVER']
        else:
            logging.error('CARLA_SERVER binary not provided!')

    def reset(self, map_id, track_id=Track.SENSORS, port=2000, human_flag=False):
        # first we check if there is need to clean up
        if self._proc is not None:
            logging.info('Stoppoing previous server [PID=%s]', self._proc.pid)
            self._proc.kill()
            self._outs, self._errs = self._proc.communicate()

        # temporary config file
        conf_file = '.tempconf.ini'

        exec_command = []
        exec_command.append(self._carla_server_binary)
        exec_command.append(map_id)

        exec_command.append('-world-port={}'.format(port))
        if not human_flag:
            exec_command.append('-benchmark')
            exec_command.append('-fps=20')

        if track_id == Track.NO_RENDERING:
            # create ini file
            with open(conf_file, 'w') as fd:
                fd.write('[CARLA / Server]\n')
                fd.write('DisableRendering = true\n')

            exec_command.append('-carla-settings={}'.format(conf_file))

        print(exec_command)
        self._proc = subprocess.Popen(exec_command, stdout=subprocess.PIPE,
                                      bufsize=1)

    def stop(self):
        self._proc.kill()
        self._outs, self._errs = self._proc.communicate()


class ServerManagerDocker(ServerManager):
    def __init__(self, opt_dict):
        super(ServerManagerDocker, self).__init__(opt_dict)

        if 'DOCKER_VERSION' in opt_dict:
            self._docker_string = '{}'.format(opt_dict['DOCKER_VERSION'])
        else:
            logging.error('Docker version not provided!')

        self._docker_id = ''


    def reset(self, map_id, track_id=Track.SENSORS, port=2000,
              human_flag=False):
        # first we check if there is need to clean up
        if self._proc is not None and self._docker_id is not '':
            logging.info('Stoppoing previous server [PID=%s]', self._proc.pid)
            self.stop()
            self._proc.kill()
            self._outs, self._errs = self._proc.communicate()

        self._docker_id = ''.join(random.choice(string.ascii_uppercase +
                                                string.digits) for _ in
                                  range(64))
        # temporary config file
        conf_file = '.tempconf.ini'

        exec_command = [ 'docker', 'run', '--name',
                         '{}'.format(self._docker_id), '-p',
                         '{}-{}:{}-{}'.format(port, port+2, port, port+2),
                         '--runtime=nvidia', '-e', 'NVIDIA_VISIBLE_DEVICES=0',
                         'carlasim/carla:{}'.format(self._docker_string),
                         '/bin/bash', 'CarlaUE4.sh']

        exec_command.append(map_id)

        exec_command.append('-world-port={}'.format(port))
        if not human_flag:
            exec_command.append('-benchmark')
            exec_command.append('-fps=20')



        print(exec_command)
        self._proc = subprocess.Popen(exec_command, stdout=subprocess.PIPE,
                                      bufsize=1)

    def stop(self):
        exec_command = ['docker', 'kill', '{}'.format(self._docker_id)]
        self._proc = subprocess.Popen(exec_command)