# CARLA, Copyright (C) 2017 Computer Vision Center (CVC)

"""Basic TCP client."""

import logging
import socket
import struct


class TCPClient(object):
    def __init__(self, host, port, timeout):
        self._host = host
        self._port = port
        self._timeout = timeout
        self._socket = None

    def connect(self):
        self._socket = socket.create_connection(address=(self._host, self._port), timeout=self._timeout)
        self._socket.settimeout(self._timeout)
        self._log('connected')

    def disconnect(self):
        if self._socket is not None:
            self._log('disconnecting...')
            self._socket.close()
            self._socket = None

    def connected(self):
        return self._socket is not None

    def write(self, message):
        if self._socket is None:
            raise RuntimeError('%s:%s not connected' % (self._host, self._port))
        header = struct.pack('<L', len(message))
        self._socket.sendall(header + message)

    def read(self):
        header = self._read_n(4)
        if not header:
            raise RuntimeError('%s:%s connection closed' % (self._host, self._port))
        length = struct.unpack('<L', header)[0]
        data = self._read_n(length)
        return data

    def _read_n(self, length):
        if self._socket is None:
            raise RuntimeError('%s:%s not connected' % (self._host, self._port))
        buf = bytes()
        while length > 0:
            data = self._socket.recv(length)
            if not data:
                raise RuntimeError('%s:%s connection closed' % (self._host, self._port))
            buf += data
            length -= len(data)
        return buf

    def _log(self, message, *args):
        logging.debug('tcpclient %s:%d - ' + message, self._host, self._port, *args)
