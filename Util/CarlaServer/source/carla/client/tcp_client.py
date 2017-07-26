# CARLA, Copyright (C) 2017 Computer Vision Center (CVC)

import logging
import socket
import struct

from contextlib import contextmanager


def _to_hex_str(header):
    return ':'.join('{:02x}'.format(ord(c)) for c in header)


class TCPClient(object):
    def __init__(self, host, port):
        self._host = host
        self._port = port
        self._socket = None

    def connect(self, timeout):
        self._socket = socket.create_connection(address=(self._host, self._port), timeout=timeout)
        self._socket.settimeout(timeout)
        self._log('connected')

    def disconnect(self):
        if self._socket is not None:
            self._socket.close()

    def write(self, message):
        header = struct.pack('<L', len(message))
        # self._log('sending (%d bytes) = {%s %s}', len(message), _to_hex_str(header), _to_hex_str(message))
        self._socket.sendall(header + message)

    def read(self):
        header = self._read_n(4)
        if header == '':
            return ''
        length = struct.unpack('<L', header)[0]
        # self._log('received header: %s (%d)', _to_hex_str(header), length)
        data = self._read_n(length)
        # self._log('received data: %s', _to_hex_str(data))
        return data

    def _read_n(self, length):
        buf = ''
        while length > 0:
            data = self._socket.recv(length)
            if data == '':
                raise RuntimeError('connection closed')
            buf += data
            length -= len(data)
        return buf

    def _log(self, message, *args):
        logging.debug('tcpclient %s:%d - ' + message, self._host, self._port, *args)


@contextmanager
def connect_to_tcp_server(host, port, timeout):
    """Context manager to create a TCPClient."""
    tcpclient = TCPClient(host, port)
    try:
        tcpclient.connect(timeout)
        yield tcpclient
    finally:
        tcpclient.disconnect()
