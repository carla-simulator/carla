# CARLA, Copyright (C) 2017 Computer Vision Center (CVC)

import logging

from contextlib import contextmanager


def to_hex_str(header):
    return ':'.join('{:02x}'.format(ord(c)) for c in header)


@contextmanager
def make_client(client_type, *args, **kwargs):
    """Context manager to create a client."""
    client = None
    try:
        client = client_type(*args, **kwargs)
        client.connect()
        yield client
    finally:
        if client is not None:
            client.disconnect()
