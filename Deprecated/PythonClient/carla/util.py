# Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma de
# Barcelona (UAB).
#
# This work is licensed under the terms of the MIT license.
# For a copy, see <https://opensource.org/licenses/MIT>.

import datetime
import sys

from contextlib import contextmanager


@contextmanager
def make_connection(client_type, *args, **kwargs):
    """Context manager to create and connect a networking client object."""
    client = None
    try:
        client = client_type(*args, **kwargs)
        client.connect()
        yield client
    finally:
        if client is not None:
            client.disconnect()


class StopWatch(object):
    def __init__(self):
        self.start = datetime.datetime.now()
        self.end = None

    def restart(self):
        self.start = datetime.datetime.now()
        self.end = None

    def stop(self):
        self.end = datetime.datetime.now()

    def seconds(self):
        return (self.end - self.start).total_seconds()

    def milliseconds(self):
        return 1000.0 * self.seconds()


def to_hex_str(header):
    return ':'.join('{:02x}'.format(ord(c)) for c in header)


if sys.version_info >= (3, 3):

    import shutil

    def print_over_same_line(text):
        terminal_width = shutil.get_terminal_size((80, 20)).columns
        empty_space = max(0, terminal_width - len(text))
        sys.stdout.write('\r' + text + empty_space * ' ')
        sys.stdout.flush()

else:

    # Workaround for older Python versions.
    def print_over_same_line(text):
        line_length = max(print_over_same_line.last_line_length, len(text))
        empty_space = max(0, line_length - len(text))
        sys.stdout.write('\r' + text + empty_space * ' ')
        sys.stdout.flush()
        print_over_same_line.last_line_length = line_length
    print_over_same_line.last_line_length = 0
