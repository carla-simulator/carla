# CARLA, Copyright (C) 2017 Computer Vision Center (CVC)

import datetime

from contextlib import contextmanager


def to_hex_str(header):
    return ':'.join('{:02x}'.format(ord(c)) for c in header)


@contextmanager
def make_connection(client_type, *args, **kwargs):
    """Context manager to create and connect a networking object."""
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

    def stop(self):
        self.end = datetime.datetime.now()

    def milliseconds(self):
        return 1000.0 * (self.end - self.start).total_seconds()
