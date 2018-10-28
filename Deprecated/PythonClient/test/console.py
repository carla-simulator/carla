# Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma de
# Barcelona (UAB).
#
# This work is licensed under the terms of the MIT license.
# For a copy, see <https://opensource.org/licenses/MIT>.

"""CARLA client console"""


import cmd
import logging
import subprocess
import tempfile
import threading
import time


from carla.client import CarlaClient
from carla.sensor import Camera, Image
from carla.settings import CarlaSettings


class _Control(object):
    def __init__(self):
        self.c_throttle = 0.0
        self.c_steer = 0.0
        self.c_brake = 0.0
        self.c_hand_brake = False
        self.c_reverse = False

    def action_list(self):
        return [x[2:] for x in dir(self) if x.startswith('c_')]

    def kwargs(self):
        return dict((x[2:], getattr(self, x)) for x in dir(self) if x.startswith('c_'))

    def set(self, line):
        control, value = line.split(' ')
        control = 'c_' + control
        if not hasattr(self, control):
            raise ValueError('invalid control: %r' % control[2:])
        setattr(self, control, self._parse(type(getattr(self, control)), value))

    @staticmethod
    def _parse(atype, value):
        if atype == bool:
            false_keys = ['f', 'false', '0', 'n', 'no', 'disable', 'off']
            return value not in false_keys
        return atype(value)


def get_default_carla_settings(args):
    settings = CarlaSettings(
        SynchronousMode=args.synchronous,
        SendNonPlayerAgentsInfo=False,
        NumberOfVehicles=20,
        NumberOfPedestrians=40,
        WeatherId=1)
    settings.add_sensor(Camera('Camera1'))
    return str(settings)


def edit_text(text):
    editor = 'vim'
    with tempfile.NamedTemporaryFile('w+', suffix='.ini') as fp:
        fp.write(text)
        fp.flush()
        try:
            if 0 != subprocess.run([editor, fp.name]).returncode:
                print('Cancelled.')
                return None
        except FileNotFoundError:
            logging.error('error opening text editor, is %r installed?', editor)
            return None
        fp.seek(0)
        return fp.read()


class CarlaClientConsole(cmd.Cmd):
    def __init__(self, args):
        cmd.Cmd.__init__(self)
        self.args = args
        self.prompt = '\x1b[1;34m%s\x1b[0m ' % '(carla)'
        self.client = CarlaClient(args.host, args.port)
        self.settings = get_default_carla_settings(args)
        self.print_measurements = False
        self.control = _Control()
        self.thread = threading.Thread(target=self._agent_thread_worker)
        self.done = False
        self.thread.start()

    def cleanup(self):
        self.do_disconnect()
        self.done = True
        if self.thread.is_alive():
            self.thread.join()

    def default(self, line):
        logging.error('unknown command \'%s\'!', line)

    def emptyline(self):
        pass

    def precmd(self, line):
        return line.strip().lower()

    def can_exit(self):
        return True

    def do_exit(self, line=None):
        """Exit the console."""
        return True

    def do_eof(self, line=None):
        """Exit the console."""
        print('exit')
        return self.do_exit(line)

    def help_help(self):
        print('usage: help [topic]')

    def do_disconnect(self, line=None):
        """Disconnect from the server."""
        self.client.disconnect()

    def do_new_episode(self, line=None):
        """Request a new episode. Connect to the server if not connected."""
        try:
            self.control = _Control()
            if not self.client.connected():
                self.client.connect()
            self.client.load_settings(self.settings)
            self.client.start_episode(0)
            logging.info('new episode started')
        except Exception as exception:
            logging.error(exception)

    def do_control(self, line=None):
        """Set control message:\nusage: control [reset|<param> <value>]\n(e.g., control throttle 0.5)"""
        try:
            if line == 'reset':
                self.control = _Control()
            else:
                self.control.set(line)
            logging.debug('control: %s', self.control.kwargs())
        except Exception as exception:
            logging.error(exception)

    def complete_control(self, text, *args, **kwargs):
        options = self.control.action_list()
        options.append('reset')
        return [x + ' ' for x in options if x.startswith(text)]

    def do_settings(self, line=None):
        """Open a text editor to edit CARLA settings."""
        result = edit_text(self.settings)
        if result is not None:
            self.settings = result

    def do_print_measurements(self, line):
        """Print received measurements to console.\nusage: print_measurements [t/f]"""
        self.print_measurements = True if not line else _Control._parse(bool, line)

    def _agent_thread_worker(self):
        filename = '_images/console/camera_{:0>3d}/image_{:0>8d}.png'
        while not self.done:
            try:
                measurements, sensor_data = self.client.read_data()
                if self.print_measurements:
                    print(measurements)

                if self.args.images_to_disk:
                    images = [x for x in sensor_data.values() if isinstance(x, Image)]
                    for n, image in enumerate(images):
                        path = filename.format(n, measurements.game_timestamp)
                        image.save_to_disk(path)
                self.client.send_control(**self.control.kwargs())
            except Exception as exception:
                # logging.exception(exception)
                time.sleep(1)
